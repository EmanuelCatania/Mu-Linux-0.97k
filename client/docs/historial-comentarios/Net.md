# Historial de comentarios: `src/Net/`

Comentarios de desarrollo movidos desde `src/Net/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Net/Crypto.cpp`

### Línea 30 — antes de `void __fastcall CSQuest_clearQuest(int param_1);`

```cpp
// Agregadas por el refactor B3: se declaraban localmente en el archivo del
// que se movieron estas funciones. Migrar a functions.h mas adelante.
```

### Línea 266 en `HashTable_Insert` — antes de `if (this_ == nullptr || *(int *)this_ == 0 || *(int *)((int)this_ + 0xc) == 0) return;`

```cpp
  // Ofuscación por HashTable: si la tabla no fue construida (vtable==NULL
  // or capacity==0) treat as no-op. CLAUDE.md marks these ops as non-game logic.
```

### Línea 413 — antes de `// Forward decl — body below.`

```cpp
// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 3155-3436 (282 lines)
// CSimpleModulus encrypt/decrypt: CSimpleModulus_Encode, FUN_0053cd20, CSimpleModulus_Decode, FUN_0053ce30
// + helpers: CSimpleModulus_EncryptBlock, CSimpleModulus_DecryptBlock, CsmTrace, CsmWatchdog
// =============================================================================
```

### Línea 700 — antes de `void __fastcall Quest_FullInit(void *param_1_raw) {`

```cpp
// ── Quest_FullInit — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 723 — antes de `// FUN_00403ef0 @ 0x00403EF0 (17 bytes) — quest vtable init`

```cpp
// ── FUN_00403ef0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 17-byte ─────────────────────────────────────────────────────────────────
```

### Línea 732 — antes de `// FUN_00403f10 @ 0x00403F10 — Quest ~dtor`

```cpp
// ── FUN_00403f10 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 30-byte: virtual destructors (deinit + conditional delete) ──────────────
```

### Línea 741 — antes de `void __cdecl Packet_DecryptBuffer(void *vparam_1, void *vparam_2) {`

```cpp
// ── Packet_DecryptBuffer — movida desde stubs_externs.cpp (refactor B3) ──
```

### Línea 771 — antes de `int  __cdecl    Cloth_Solve(DWORD *a1);`

```cpp
// ── Cloth_Solve — movida desde stubs_externs.cpp (refactor B3) ──
```

### Línea 776 — antes de `int __cdecl CSQuest_ProceedButton(void *param_1) {`

```cpp
// 2026-08-22: acá había un resumen con SÓLO la rama del botón de cerrar.
// Faltaba la del botón de aceptar/continuar la quest — el que sub_403320
// dibuja en (485,355) 120x24 con GlobalText[699] ("Proceder con la quest").
// O sea el botón se veía y hasta se pintaba al pasar el mouse (ese feedback
// está en sub_403320), pero el click no mandaba nada y la quest no avanzaba.
```

### Línea 836 — antes de `// ═══════════════════════════════════════════════════════════════════════════════`

```cpp
// ── FUN_00408ff0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 10
// ═══════════════════════════════════════════════════════════════════════════════
```

### Línea 882 — antes de `void __fastcall FUN_004090b0(void* ecx, void* /*edx*/, int param_1, float param_2, int par`

```cpp
// ── FUN_004090b0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 921 — antes de `void __fastcall FUN_004091d0(void* ecx, void* /*edx*/, int param_1, int param_2, float par`

```cpp
// ── FUN_004091d0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 944 — antes de `void __cdecl VerletNode_AddToSystem(void *widget, float p1, float p2, float p3, float radi`

```cpp
// ── VerletNode_AddToSystem — movida desde stubs_externs.cpp (refactor B3) ──
// VerletNode_AddToSystem @ 0x00409250 — VerletNode_AddToSystem: allocate node, init, insert into doubly-linked list.
// La cabeza de la lista está en this+0x50 (nodo de 0xc bytes: [0]=datos, [4]=next, [8]=prev). El contador en this+0x48.
// La entrada nueva se inserta justo después del centinela de cabecera.
```

### Línea 956 en `VerletNode_AddToSystem` — antes de `int *entry = (int *)operator_new(0xc);`

```cpp
    // aloca la entrada de la lista enlazada (0xc bytes: [+0]=node_ptr, [+4]=next, [+8]=prev)
```

### Línea 964 en `VerletNode_AddToSystem` — antes de `int tail = *(int *)(thiz + 0x50);      // this[20]`

```cpp
        // 2026-08-11 FIX (crash 0xC0000005 escribiendo a 0xCDCDCDD5 = heap sin
        // inicializar + 8, al crear la capa del MG): esta rutina trataba
        // `thiz + 0x50` como si el centinela estuviera EMBEBIDO ahí, y además
        // asumía `+4 = next / +8 = prev`. Las dos cosas están al revés.
        // IDA `sub_409250` L27-32:
        //     result[1] = *(_DWORD *)(this[20] + 4);
        //     *(_DWORD *)(*(_DWORD *)(this[20] + 4) + 8) = result;
        //     result[2] = this[20];
        //     *(_DWORD *)(this[20] + 4) = result;
        //     this[18] = this[18] + 1;
        // `this[20]` es el VALOR del puntero al centinela de cola que guardó
        // `sub_407FE0` en +0x50, y el layout es **+4 = prev, +8 = next**
        // (coherente con el ctor: `head[8] = tail`, `tail[4] = head`).
        // Se inserta al FINAL, antes de la cola.
```

### Línea 988 — antes de `int   __cdecl    Cloth_CollideAnchors(DWORD *thiz);                       // colisión con `

```cpp
// ── Cloth_CollideAnchors — movida desde stubs_misc_helpers.cpp (refactor B3) ──
```

### Línea 1070 — antes de `void* __fastcall Widget_Ctor(void *param_1)`

```cpp
// ── Widget_Ctor — movida desde stubs_externs.cpp (refactor B3) ──
```

### Línea 1080 — antes de `void __fastcall FUN_004093c0(void *This) {`

```cpp
// ── FUN_004093c0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1087 — antes de `void __cdecl SpringMesh_Create(void *widget, int entity, short *slot, int type, int radius`

```cpp
// ── SpringMesh_Create — movida desde stubs_externs.cpp (refactor B3) ──
```

### Línea 1172 — antes de `void* __fastcall FUN_00409ad0(void* param_1) {`

```cpp
// ── FUN_00409ad0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// La etiqueta vieja ("CSQuest constructor") era incorrecta.
```

### Línea 1194 — antes de `void __fastcall scalar_deleting_destructor_locale(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── scalar_deleting_destructor_locale — movida desde stubs_bulk_small.cpp (refactor B3) ──
// scalar_deleting_destructor_locale @ 0x00409B60 — SoundWidgetB ~dtor
```

### Línea 1201 — antes de `void __fastcall Locimp_dtor(void* param_1) {`

```cpp
// ── Locimp_dtor — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// Locimp_dtor @ 0x00409B80 (~39 lines) — CSQuest destructor: clear list + free sentinels
// __fastcall(ecx=questObj). Setea la vtable, llama a FUN_00409d20 (limpia todos los nodos),
// y después libera la cadena de nodos entre la cabeza y la cola, y los propios centinelas.
```

### Línea 1207 en `Locimp_dtor`

```cpp
    // FUN_00409d20(param_1) — clear all quest nodes
```

### Línea 1213 — antes de `void __fastcall FUN_00409d20(int param_1) {`

```cpp
// ── FUN_00409d20 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00409d20 @ 0x00409D20 (~34 lines) — CSQuest: clear all quest nodes from linked list
// __fastcall(ecx=questObj). Itera desde head->next hasta tail, llama a Widget_Release sobre los datos
// de cada nodo y después invoca el destructor vía la vtable. Libera todos los nodos intermedios.
```

### Línea 1225 — antes de `void __fastcall LinkedList_DestroyAll(int *param_1) {`

```cpp
// ── LinkedList_DestroyAll — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 1242 — antes de `void FUN_00409ea0(void) { LinkedList_InitSentinels((void *)&DAT_00590b00); }`

```cpp
// ── FUN_00409ea0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1246 — antes de `void FUN_00409eb0(void) {}`

```cpp
// ── FUN_00409eb0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1250 — antes de `void* __fastcall WidgetB_Ctor(void *param_1)`

```cpp
// ── WidgetB_Ctor — movida desde stubs_misc_helpers.cpp (refactor B3) ──
```

### Línea 1259 — antes de `void __fastcall FUN_00409ef0(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── FUN_00409ef0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1266 — antes de `void __fastcall WidgetB_SetVtable(void *param_1)`

```cpp
// ── WidgetB_SetVtable — movida desde stubs_misc_helpers.cpp (refactor B3) ──
```

### Línea 1274 — antes de `// WidgetB_ZeroFields @ 0x00409F20 — WidgetB_ZeroFields: clear 4 fields (+4,+8,+0x18,+0x1c`

```cpp
// ── WidgetB_ZeroFields — movida desde stubs_misc_helpers.cpp (refactor B3) ──
```

### Línea 1286 — antes de `int __fastcall FUN_00409f30(void* ecx, void* /*edx*/, int param_1, int param_2, int param_`

```cpp
// ── FUN_00409f30 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1304 — antes de `void __fastcall FUN_0040a0a0(void *This, int /*edx*/, int param_1, int param_2, int param_`

```cpp
// ── FUN_0040a0a0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 1318 — antes de `void __fastcall FUN_0040a110(void* ecx, void* /*edx*/, short param_1, short param_2, short`

```cpp
// ── FUN_0040a110 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1336 — antes de `void __fastcall FUN_0040a1c0(void* ecx, void* /*edx*/, short param_1, int param_2, short p`

```cpp
// ── FUN_0040a1c0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1353 — antes de `void __fastcall FUN_0040a300(void* ecx, void* /*edx*/, int param_1) {`

```cpp
// ── FUN_0040a300 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1371 — antes de `void __fastcall FUN_004236c0(void *This, int /*edx*/, int *param_1) {`

```cpp
// ── FUN_004236c0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
```

### Línea 1389 — antes de `void FUN_00423c30(void) { CWsctlc_Close(((int)(uintptr_t)SocketClient)); }`

```cpp
// ── FUN_00423c30 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1393 — antes de `void __cdecl InitGuildWar(void) {`

```cpp
// ── InitGuildWar — movida desde stubs_externs.cpp (refactor B3) ──
```

### Línea 1416 — antes de `void __cdecl CWsctlc_Close(int ctx) {`

```cpp
// ── CWsctlc_Close — movida desde stubs_linker.cpp (refactor B3) ──
```

### Línea 1426 — antes de `void __fastcall FUN_0053cbb0(int *param_1) {`

```cpp
// ── FUN_0053cbb0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1433 — antes de `void __fastcall FUN_0053cbd0(int ecx, int /*edx*/, BYTE param_1) {`

```cpp
// ── FUN_0053cbd0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1440 — antes de `void __fastcall FUN_0053cbf0(int *param_1) { *param_1 = (int)&PTR_FUN_0055389c; }`

```cpp
// ── FUN_0053cbf0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1444 — antes de `unsigned int __cdecl FUN_0053ce30(void *self, unsigned short *param_1, int param_2) {`

```cpp
// ── FUN_0053ce30 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1455 — antes de `int __stdcall CSimpleModulus_AddBits(int a1, unsigned int a2, int a3, unsigned int a4, int`

```cpp
// ── CSimpleModulus_AddBits — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1497 — antes de `void __stdcall CSimpleModulus_Shift(unsigned char *a1, int a2, int a3)`

```cpp
// ── CSimpleModulus_Shift — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1535 — antes de `// CSimpleModulus_GetByteOfBit @ 0x0053D170 (10 bytes)`

```cpp
// ── CSimpleModulus_GetByteOfBit — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 1539 — antes de `int __cdecl CSimpleModulus_GetByteOfBit(int a1)`

```cpp
// CSimpleModulus_GetByteOfBit @ 0x0053D170 (10 bytes)
// CSimpleModulus_GetByteOfBit (IDA-activated, was Ghidra stub)
```

### Línea 1546 — antes de `int __cdecl CSimpleModulus_LoadKey(void *self, const char *filename, short magic,`

```cpp
// ── CSimpleModulus_LoadKey — movida desde stubs_bulk_misc.cpp (refactor B3) ──
```

### Línea 1600 — antes de `static void __cdecl FUN_0053cc00_impl(int param_1) {`

```cpp
// -- FUN_0053cc00_impl: helper local, movido desde stubs_bulk_small.cpp (B3) --
```

## `src/Net/MuEmu.cpp`

### Línea 78 — antes de `static void DumpHex(const char* tag, const BYTE* buf, int len)`

```cpp
// -----------------------------------------------------------------------------
// Hex/ASCII dump helper for diagnostics (first/last N bytes of a buffer).
//
// 2026-04-29: gated behind MUEMU_TRACE.  When debugging under VS the high
// volume of hex bytes flowing through DbgLog → WriteFile → debug.log was
// triggering first-chance KernelBase AVs (ImePadServer-style) and flooding
// the log file (300 KB+ of recv/send dumps per minute).  Define
// MUEMU_TRACE in the project for opt-in tracing during net-protocol work.
// -----------------------------------------------------------------------------
```

## `src/Net/MuEmu.h`

### Línea 1 — antes de `#pragma once`

```cpp
// MuEmu.h
// MuEmu-compat layer. Strictly isolated from the reversed vanilla-0.97k code.
//
// What this server actually does
// ------------------------------
// This Linux MuEmu port enables server-side "HackCheck" encryption
// (ENCRYPT_STATE=1 in GameServer/stdafx.h) which wraps EVERY outbound byte with:
//
//     enc[n] = (plain[n] + K) ^ K1   (mod 256, all 8-bit arithmetic)
//
// where K1 = EncDecKey1 and K = EncDecKey2 * EncDecKey1. The symmetric inverse is:
//
//     plain[n] = (enc[n] ^ K1) - K   (mod 256)
//
// The key is derived from gServerInfo.m_CustomerName XOR m_ServerSerial at
// server init time, so it's a per-install constant.  For this specific server
// (ServerSerial="TbYehR2hFUPBKgZj") we reverse-engineered the effective key
// empirically from the observed handshake:
//
//     Expected plaintext : C1 0C F1 00 01 23 XX 30 39 37 31 31   (C1:F1:00 connect-client)
//     Observed ciphertext: 41 0C 71 00 01 27 YY 30 39 3B 31 31
//
// The only key that makes every byte match under `(x^K)-K` is K1=K=0x42
// (equivalent class {0x42,0xC2} — both produce identical bytes under the
// symmetric transform).
//
// Architecture
// ------------
// Net_Recv (stubs.cpp CWsctlc_nRecv) calls MuEmu::DecryptRecv on every fresh
// chunk returned by recv(), BEFORE any C1/C2/C3/C4 parsing. Likewise any
// outbound send site that talks to a MuEmu server should encrypt with
// MuEmu::EncryptSend just before calling ::send().
//
// The old "0x41 preamble intake" approach was WRONG — the 0x41 wasn't a
// special preamble, it was a regular C1 header with encryption applied.
```

### Línea 42 — antes de `constexpr BYTE kEncKey1Default   = 0x42;   // EncDecKey1            (mascara XOR)`

```cpp
// -----------------------------------------------------------------------------
// Clave efectiva, derivada en runtime (2026-08-26)
// -----------------------------------------------------------------------------
// Antes estas dos constantes estaban HARDCODEADAS en 0x42/0x42, que es lo que
// da la formula del server para CustomerName="MuLinux" + ServerSerial=
// "TbYehR2hFUPBKgZj". Contra cualquier otro CustomerName el cliente conectaba
// pero desencriptaba basura y nunca reconocia el JoinServer (F1/00): se quedaba
// en "conectando al GameServer" para siempre.
//
// Ahora `InitKeys` reproduce la derivacion del server y `server.cfg` puede
// traer CustomerName/ServerSerial. Sin esas lineas quedan los valores de abajo,
// asi que los server.cfg viejos siguen funcionando igual.
//
// Equivalencia util al comparar contra el binario: sumar 0x80 mod 256 es lo
// mismo que XOR 0x80, asi que (K1,K)=(0x42,0x42) y (0xC2,0xC2) producen bytes
// identicos. La formula da 0xC2; el valor historico de este archivo era 0x42.
```

## `src/Net/Net_Bux.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Net_Bux.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

### Línea 19 — antes de `void MapFileDecrypt(BYTE* buf, int size)`

```cpp
// MapFileDecrypt — decrypt EncTerrain%d.{map,obj} with MU map-file algorithm.
// Reference: público en source leaks de Mu Online Season clients (válido para 0.97.x).
// Algorithm: each byte is XOR'd with a 16-byte rolling key, minus a per-byte
// running counter (wKey) that is updated based on the original ciphertext byte.
//   plain[i] = (cipher[i] ^ MapFileKey[i & 0x0F]) - wKey
//   wKey = cipher[i] + 0x3D    (note: cipher byte, not plain)
// Initial wKey = 0x5E.
//
// BUG-FIX 2026-05-01: BuxConvert_1 (3-byte XOR) NO sirve para .map/.obj — esos
// archivos usan un algorithm distinto (de ahí el prefix "Enc" mientras que .att
// usa BuxConvert_1 simple).
```

## `src/Net/Net_CharSelect.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Net_CharSelect.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

## `src/Net/Net_Connect.cpp`

### Línea 209 — antes de `extern "C" int __cdecl CWsctlc_Connect(DWORD This, const char* ip_addr,`

```cpp
// =============================================================================
// IDA: CWsctlc::Connect (0x0043DCD0)
// (251 bytes per IDA decompile raw/0043DCD0_CWsctlc_Connect.c).
//
// Lower-level TCP connect helper used by CWsctlc socket sessions. The `This`
// pointer is a CWsctlc instance with layout:
//   This[+0]  HWND  hWnd  (the window that will receive WSAAsyncSelect events)
//   This[+8]  SOCKET socket descriptor (created earlier by Net_Connect_Server)
//
// Behavior:
//   1. Validate hWnd != 0; otherwise show error MessageBox + return 0.
//   2. Resolve ip_addr: try inet_addr first, fall back to gethostbyname.
//   3. Call connect(); accept either success or WSAEWOULDBLOCK (10035).
//   4. Register WSAAsyncSelect for FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE
//      (= 0x23 = bit pattern from IDA constant).
//
// Returns:
//   1 = connect ok (or pending non-blocking)
//   2 = host name resolution failure
//   0 = hWnd null OR connect failed -> closesocket
//
// 2026-05-08: ported as part of the companion-DLL Offsets.h cross-reference.
// Our existing Net_Connect_Server (CreateSocket) handles the higher-level
// connect flow including hash-table bootstrap; CWsctlc::Connect is the inner
// connect+select helper.  Currently no caller in our build references it
// directly (we connect via different code paths) but the symbol is exported
// for completeness.
// =============================================================================
// IDA: CWsctlc::Connect (0x0043DCD0)
```

## `src/Net/Net_Disconnect.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Net_Disconnect.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

## `src/Net/Net_Events.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Net_Events.cpp
// Handlers de los opcodes 0x90-0x99 — eventos (Devil Square, Blood Castle,
// Golden Archer / Event Chip) y migracion de server.
//
// Por que existe este archivo
// ---------------------------
// Hasta 2026-08-26 estos opcodes estaban despachados en Net_Process.cpp a
// handlers de GUILD inventados por el port (Guild_CreateOk,
// Guild_AddMemberResult, Guild_MemberList, ...). Los nombres que IDA le da a
// las funciones del binario original desmienten esa etiqueta una por una:
//
//   opcode | funcion real en el binario           | etiqueta que tenia el port
//   -------|--------------------------------------|---------------------------
//   0x90   | ReceiveMoveToDevilSquareResult 436820 | "Guild create result"
//   0x91   | ReceiveEventZoneOpenTime       436CB0 | "Guild add member result"
//   0x92   | StartMatchCountDown            47EC00 | (no tenia case)
//   0x93   | ReceiveDevilSquareRank         436A80 | "Guild member list"
//   0x94   | ReceiveEventChipInfomation     4372C0 | "Guild char-select result"
//   0x95   | ReceiveEventChip               437380 | "Guild update pos"
//   0x96   | ReceiveMutoNumber              4373A0 | "Guild set target pos"
//   0x99   | ReceiveServerImmigration       4373D0 | "Guild join toggle"
//
// Y MuEmu coincide con IDA en los ocho: manda 0x90 desde DevilSquare.cpp, 0x91
// desde Protocol.cpp (PMSG_EVENT_REMAIN_TIME_SEND), 0x92/0x93 desde
// DevilSquare.cpp y BloodCastle.cpp, y 0x94-0x97 desde GoldenArcher.cpp. El
// guild real vive en 0x50-0x62 (Guild.cpp del server), que el cliente ya
// atiende aparte y correctamente.
//
// O sea no habia que elegir entre fidelidad a IDA y fidelidad a MuEmu: las dos
// fuentes dicen lo mismo y el port estaba mal. Los Guild_* de Party.cpp quedan
// sin callers (ver la nota alli).
//
// Hasta que se abra algun evento del lado del server esto no cambia nada
// visible, salvo el 0x91, que es la respuesta al click derecho sobre las
// entradas de evento.
```

## `src/Net/Net_LegacyAuth.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Net_LegacyAuth.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

## `src/Net/Net_LegacyRuntime.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Net_LegacyRuntime.cpp
//
// Final network/session functions extracted from stubs_game.cpp.  The code
// retains each IDA symbol/address in its leading comment; this relocation does
// not change packet, initialization, or anti-tamper behaviour.
```

### Línea 17 — antes de `extern "C" BYTE OffsetInventoryItems[];   // HUD_Pass3.cpp — main inv (8x8 + 12 wear)`

```cpp
// 2026-05-08 BUG-FIX MAYÚSCULO: en el binario original `OffsetInventoryItems`
// y `DAT_07ea8410` son el mismo símbolo (idem para Mix/Warehouse). En nuestra
// build son globals separados, así que comparaciones tipo
// `DAT_07ea9800 == &DAT_07ea8410` siempre fallaban → drop dispatcher tomaba
// el fallback "Other context" y enviaba srcType=3 con srcIdx==dstIdx, lo cual
// era no-op del lado del server y no liberaba el lock EnableUse.
// Forward-decls a nivel TU para usar las direcciones reales del pool.
```

### Línea 33 — antes de `#ifndef LODWORD`

```cpp
// IDA Hex-Rays intrinsic shims (mirror of stubs.cpp shims).
```

### Línea 80 en `InitGame` — antes de `DAT_05826d24 = 0;     // SummonLife`

```cpp
    // IDA InitGame L31 es `SummonLife = 0`, y SummonLife vive en 0x05826D24
    // (verificado con ida_xrefs_to: lo escriben InitGame, ReceiveRevival x3 y
    // ProtocolCore, y lo lee RenderEquipedHelperLife).  El port escribia
    // DAT_07E11D28, que es **MouseUpdateTime** -- el contador del debounce de
    // movimiento de Player_InputTick.  Dos efectos: SummonLife nunca se
    // limpiaba al salir de la sesion, y ponerlo en 0 aca hacia que al volver
    // al mundo el primer click quedara bloqueado hasta contar de nuevo hasta
    // MouseUpdateTimeMax (que InitGame no toca y puede venir en ~28-49 del
    // ultimo camino recorrido) = hasta ~2 s sin poder caminar.
```

### Línea 100 en `InitGame` — antes de `DAT_07eaa160 = 0;     // CheckInventory`

```cpp
    // IDA InitGame L40 es `CheckInventory = 0`, y CheckInventory vive en
    // 0x07EAA160 -- es el puntero al ITEM bajo el mouse que Scene_MapTick le
    // pasa a RenderItemInfo para dibujar el tooltip.  El port limpiaba
    // DAT_07E11D24, que es otro global (el tipo de item de la ventana F1, el
    // que indexa sub_4C2E20).  Es el mismo error de alias que ya se habia
    // corregido en Item_ClickHandler.cpp en 2026-08-22, aca sin corregir.
    //
    // Efecto: al salir del mundo el tooltip NO se limpiaba y se quedaba
    // dibujado encima del char-select y del select-server (reportado
    // 2026-09-27: "un tooltip llego hasta el login").
```

### Línea 115 en `InitGame` — antes de `World = -1;   // World`

```cpp
    // IDA InitGame L41 es `World = -1`, y World es 0x0055A7AC (World).
    // El port escribia DAT_005615c4, que es g_lpszMp3[0] — el puntero al mp3 de
    // la taberna — asi que cada InitGame lo dejaba en -1 y PlayMp3 recibia (char*)-1.
```

### Línea 119 en `InitGame` — antes de `DAT_07e11d6f = 0;     // LockInputStatus`

```cpp
    // CSQuest__ClearQuest(g_csQuest);
    // IDA InitGame L43 es `LockInputStatus = 0`, y LockInputStatus vive en
    // 0x07E11D6F (xrefs: WndProc x4, InitGame, ReceiveJoinMapServer,
    // RenderIME_Status).  El port escribia DAT_07E11D1C, que es **LoadingWorld**
    // -- el contador que gatea el frame de render (`if (LoadingWorld > 30) return`).
```

### Línea 162 — antes de `void __cdecl ReceiveChat(BYTE *ReceiveBuffer)`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// ReceiveChat @ 0x00427630 (692 bytes, ~156 lines)
// Packet handler for opcode 0x00 (chat message from server).
// Packet layout: [C1][len][00][sender:10][msg:60][type:1][text...]
//
// If SceneFlag == 2 (in-game):
//   Sends a 4-byte ACK packet {0xC1, 0x04, 0x0E, ...} back to server
//   (keep-alive/chat ACK). Handles WSAEWOULDBLOCK by queuing to send buffer.
//
// Otherwise (login/charselect scene):
//   Extracts sender name (10 bytes @ offset 3) and message text (59 bytes @ offset 0xE).
//   Routes by chat type byte at offset 0x0D:
//     '~' (0x7E) → AddText(sender, msg, 4)        — whisper
//     '@' (0x40) → AddText(sender, msg, 5)        — GM/announce
//     '#' (0x23) → AssignChat(sender, msg, 1)     — party chat
//     default    → AssignChat(sender, msg, 0) + AddText(sender, msg, 3) — normal chat
// ─────────────────────────────────────────────────────────────────────────────
```

### Línea 181 en `ReceiveChat` — antes de `if (SceneFlag == 2) {  // SceneFlag == 2 (Login scene)`

```cpp
    // BUG-FIX: DAT_07e11980 no existe en PE. SceneFlag real = SceneFlag.
    // El comentario "(in-game)" era incorrecto: 2 = Login en este cliente.
```

### Línea 218 en `ReceiveChat` — antes de `char sender[11];`

```cpp
    // --- Chat message processing (login/charselect scene) ---
```

### Línea 230 en `ReceiveChat` — antes de `extern void __cdecl UIChatLogWindow_AddText(const char* label, const char* msg, int mode);`

```cpp
    // FIX 2026-07-19: las 3 ramas llamaban `FUN_00481a40(0, sender, 0)` — función y
    // argumentos equivocados, y el canal siempre 0. IDA ReceiveChat (0x427630) usa
    // UIChatLogWindow_AddText(strID, strText, <canal>) con el canal correcto por
    // prefijo. Sin esto el mensaje nunca entraba al chat log con su color/canal.
```

### Línea 377 — antes de `void __stdcall SendCheck(void)`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// SendCheck @ 0x004220A0 (2988 bytes, 631 lines)
//
// Anti-tamper/checksum packet sent periodically to the game server.
// Builds a C1/C2 packet containing XOR-encrypted character stats,
// then sends it over the game socket with WSAEWOULDBLOCK queue handling.
//
// == Packet construction ==
//
// 1. Starts with header: [C1][len][F3][03]
// 2. Appends a NULL byte (encrypted with XOR key)
// 3. Appends GetTickCount() as 4-byte DWORD (XOR-encrypted)
// 4. Hash table lookup for CharacterMachine — anti-tamper obfuscation:
//    - If not found: allocate 0x585-byte entry, insert via HashTable_Insert
//    - If found: increment refcount, clone+encrypt if refcount < 2
// 5. Reads CharacterAttribute fields and appends (XOR-encrypted):
//    - If AbilityTime[0] bit 0 == 0 (normal):
//        AttackDamageMinRight (2 bytes)
//        MagicDamageMax       (2 bytes)
//    - If AbilityTime[0] bit 0 == 1 (buff active):
//        AttackDamageMinRight - 0x14 (2 bytes)
//        MagicDamageMax - 0x14       (2 bytes)
// 6. Second hash table pass: decrement refcount, free if zero
// 7. Sets packet length field (C1 = 1-byte len, C2 = 2-byte len)
// 8. Copies packet to abStack_40c buffer
// 9. Appends random byte (rand()) at end
// 10. Hash table lookup for g_byPacketSerialSend:
//     - Inserts serial number at offset [1] or [2] depending on header type
//     - Increments g_byPacketSerialSend
//     - Decrements refcount on old serial entry
// 11. Calls CSimpleModulus_Encode to encode/compress the payload
// 12. Sends via send() with full WSAEWOULDBLOCK queue handling
//     - C1 header: acStack_914 buffer (< 0x100 encoded size)
//     - C2 header: acStack_810 buffer (>= 0x100 encoded size)
// 13. Sets DAT_05826cfc = 1, DAT_05826d00 = tickCount on first call
//
// == XOR encryption pattern (repeated ~8 times in function) ==
//
// Key: 32-byte table (same as login XOR key):
//   {0xE7,0x6D,0x3A,0x89,0xBC,0xB2,0x9F,0x73,
//    0x23,0xA8,0xFE,0xB6,0x49,0x5D,0x39,0x5D,
//    0x8A,0xCB,0x63,0x8D,0xEA,0x7D,0x2B,0x5F,
//    0xC3,0xB1,0xE9,0x83,0x29,0x51,0xE8,0x56}
//
// For each byte at position i:
//   buf[i] ^= key[i % 32] ^ buf[i+1]  (chained XOR)
//
// The key is re-initialized (forward order, then reverse order) around
// each XOR loop — this is a compiler artifact / anti-tamper pattern,
// not meaningful crypto variation (see CLAUDE.md notes).
//
// == Hash table operations ==
//
// Uses MAIN_HASH_CLASS (anti-tamper obfuscation, not game logic):
//   FUN_004041e0 — hash lookup (returns slot index or 0xFFFFFFFF)
//   HashTable_Insert — hash insert (allocates 0x585-byte entry)
//   HashTable_GetNode — hash get value (returns entry pointer)
//   Packet_DecryptByte — hash clone+encrypt entry
//   Packet_EncryptBuffer — hash free entry (when refcount hits 0)
//   Packet_EncryptByte — hash remove entry
//
// == Network send ==
//
// Same pattern as all other packet sends:
//   send() in a loop, handle WSAEWOULDBLOCK by copying to
//   SocketClientSendBuffer queue (max 0x2001 bytes), or disconnect
//   via CWsctlc_Close on hard error.
//   FUN_0043de60 called after successful partial send if
//   SocketClientLogPrint != 0 (flush pending queue).
// ─────────────────────────────────────────────────────────────────────────────
```

### Línea 486 en `SendCheck` — antes de `// --- Append character stats (XOR-encrypted) ---`

```cpp
    // --- Hash table anti-tamper: lookup CharacterMachine key ---
    // (obfuscation pattern — see CLAUDE.md anti-tamper notes)
    // Inserts/clones CharacterAttribute data into hash table,
    // XOR-encrypts the 0x584-byte block with secondary key.
```

## `src/Net/Net_PipeQuery.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Net_PipeQuery.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

## `src/Net/Net_Process.cpp`

### Línea 6 — antes de `#include "stdafx.h"`

```cpp
// Dispatcher de paquetes entrantes server→cliente. Corre en un loop do-while.
// Cada iteración desencola un paquete vía Net_GetFreeBuffer, despacha por opcode,
// y sigue hasta vaciar el pool.
//
// 72 phantom param_N / in_stack_ "parameters" are anti-tamper obfuscation noise.
//
// ── PACKET FORMAT ─────────────────────────────────────────────────────────────
//
//   Header byte 0:
//     0xC1 = 1-byte length (byte[1] = total len)
//     0xC2 = 2-byte length (byte[1-2] = len, big-endian)
//     0xC3 = encrypted 1-byte length
//     0xC4 = encrypted 2-byte length
//
//   After header:
//     byte[opcode_offset] = main opcode
//       C1: byte[2] = opcode,  byte[3..] = payload
//       C2: byte[3] = opcode,  byte[4..] = payload
//
//   Decryption: CSimpleModulus_Decode(&g_SimpleModulusSC, ...) — RC4-like stream cipher for C3/C4
//
// ── BUFFER POOL ───────────────────────────────────────────────────────────────
//
//   Net_GetFreeBuffer @ 0x0043E010
//     Pool base: SocketClient (300 entries, stride 0x2008)
//     Status flag: entry+0x401C (0 = free)
//     Returns: entry+0x4024 (packet data start), or NULL
//
//   puVar8   = current packet pointer
//   puVar9   = packet length (ushort)
//   param_1  = opcode (main switch key)
//
//   TotalPacketSize += byte[opcode] each packet (running byte counter)
//
// ── MAIN OPCODE SWITCH (param_1 = opcode byte) ────────────────────────────────
//
//   case 0x00:  Net_SendPacket(puVar8)         — re-queue/echo packet
//
//   case 0x01:  entity = FindCharacterIndex(byte[3]*256 + byte[2])
//               CreateChat(entity+0x1c1, puVar8+5, entity, 0, -1)
//               → entity name/class update (entity stride 0x394 at DAT_07abf5d0)
//
//   case 0x02:  World-enter / spawn position:
//               Copies position payload into locals (0xf dwords)
//               FUN_004801c0()           — world state init
//               if m_bWhisperSound: PlayBuffer(0x26, 0, 0)
//               UIChatLogWindow_AddText(posData, nameData, 0)
//
//   case 0x03:  XOR handshake:
//               32-byte key = {0xe7,0x6d,0x3a,0x89,...} (same global key)
//               FUN_00412de0(byte[2])    — process auth challenge byte
//               Builds response + send() with full WSAEWOULDBLOCK retry path
//
//   case 0x07:  Entity flag update:
//               entity = FindCharacterIndex(byte[3]*256 + byte[7])
//               if byte[3]==1 && !(entity->flags & byte[2]): FUN_0043bde0(byte[2], entity)
//               else: FUN_0043c070(flags, entity)
//
//   case 0x0b:  Packet buffer slot management (max 9 slots, DAT_07e11db4):
//               if DAT_07e11db4 > 9: shift buffer array down
//               Guarda puVar8 en DAT_07e016c8[slot], copia los datos a DAT_07e109cc[slot*0x100]
//               FUN_00500a80()           — process buffered data
//
//   case 0x0c:  if byte[3]==0: UIChatLogWindow_AddText(ChatWhisperID, DAT_07d4d1fc, 2)
//                              → draw login/char-select widget
//
//   case 0x0d:  FUN_00427a00(puVar8)
//
//   case 0x0f:  DAT_07c74ae0 = (byte[3] & 0xf) != 0 ? (byte[3]&0xf)*6 : 0
//               → chat/channel color/type mapping
//
//   case 0x10:  FUN_00427b90(puVar8)
//   case 0x11:  FUN_00427f40(puVar8)
//
//   case 0x12:  El mismo manejo de slot-buffer que 0x0b
//               FUN_00429690(puVar8, puVar9)
//
//   case 0x13:  FUN_0042a230(puVar8, ..., puVar9)
//   case 0x14:  Loop DeleteCharacter(entityId) por la cantidad en byte[3] — lista de destrucción del viewport
//   case 0x15:  FUN_0042acc0(puVar8)
//   case 0x16:  FUN_0042db60(puVar8, iVar20)
//   case 0x17:  FUN_0042f030(puVar8)
//   case 0x18:  FUN_0042b4f0(puVar8)
//   case 0x19:  Skills_PacketHandler(puVar8, puVar9, iVar20)
//   case 0x1a:  FUN_0042d780(puVar8)
//
//   case 0x1b:  Entity state switch on byte[3]:
//               entity = FindCharacterIndex(entityId)
//               1  → FUN_0043c070(flag_1, entity)
//               7  → FUN_0043c070(flag_2, entity)
//               0x10→ FUN_0043c070(0x100, entity)
//               0x40→ FUN_0043c070(0x40,  entity)
//
//   case 0x1c:  Manejo de slot buffer (máx 9, igual que 0x0b)
//               FUN_00428210(puVar8, iVar20)
//
//   case 0x1e:  FUN_0042cd10(puVar8, puVar9, iVar20)
//   case 0x1f:  FUN_0042a530(puVar8)
//   case 0x20:  FUN_0042f240(puVar8)
//
//   case 0x21:  Entity removal list:
//               Loop on byte[2] count: entityId = byte[5+n*2]*256 + byte[4+n*2]
//               if entityId < 1000: DAT_07e12840[entityId*0x204] = 0
//
//   case 0x22:  FUN_0042f360(puVar8)
//   case 0x23:  FUN_0042f690(puVar8)
//
//   case 0x24:  Manejo de slot buffer (igual que 0x1c)
//               FUN_0042f9a0(puVar8, iVar20)
//
//   case 0x25:  FUN_00429230(puVar8)
//   case 0x26:  FUN_00431780()          — equip item response handler
//   case 0x27:  FUN_00431a90()
//
//   case 0x28:  FUN_004cce00(byte[3], &DAT_07ea8410, 8)   — item table slot update
//               if byte[2] != 0: DAT_05826d1c = 0         — reset equip cooldown
//
//   case 0x29:  FUN_004321f0(puVar8, iVar20)
//
//   case 0x2a:  Slot buffer management
//               FUN_00431ea0(puVar8)
//
//   case 0x2c:  FUN_00437f10(puVar8)
//   case 0x30:  FUN_004301b0(puVar8, iVar20)
//   case 0x31:  FUN_00427560(puVar8)
//
//   case 0x32:  InsertInventoryItem(&DAT_07ea8410, 8, 8, byte[3], puVar8+2, 0)
//               PlayBuffer(0x1d, 0, 0)    — inventory update + UI refresh
//
//   case 0x33:  if byte[3] != 0:
//                 DAT_07e91388 = 0
//                 STRUCT_DECRYPT(&MAIN_HASH_CLASS, DAT_07cf1ffc)  — decode g_CharData
//                 DAT_07cf1ffc[0x152] = *(puVar8+2)
//                 STRUCT_ENCRYPT(&MAIN_HASH_CLASS, puVar23)       — re-encode g_CharData
//                 PlayBuffer(0x1d, 0, 0)
//
//   case 0x34:  if packet[2] != 0:
//                 STRUCT_DECRYPT; DAT_07cf1ffc[0x152] = *(puVar8+2)
//                 CalculateAll(puVar23)
//                 STRUCT_ENCRYPT; PlayBuffer(0x25, 0, 0)
//
//   case 0x36:  Server-triggered re-login:
//               Stores PIN buffer: DAT_07ea9834/38/3c ← puVar8[3/7/0xb]
//               DAT_07ea983e = 0
//               SetErrorMessage(0x79)  — ShowErrorDialog(0x79)
//               Después (si DAT_07e91388 >= 1) corta, si no: cae al 0x37
//               NOTA: el bloque XOR de envío de acá (líneas 655-840) es el camino de
//               respuesta al NACK de re-login del server — misma clave de 32 bytes, mismo loop de reintento.
//
//   case 0x37:  FUN_004332e0(puVar8)
//
//   case 0x38:  FUN_004cce00(byte[3], &DAT_07ea5298, 8)
//               PlayBuffer(0x1d, 0, 0)   — secondary inventory (bag/warehouse?)
//
//   case 0x39:  InsertInventoryItem(&DAT_07ea5298, 8, 4, byte[3], puVar8+2, 1)
//               PlayBuffer(0x1d, 0, 0)
//
//   case 0x3a:  DAT_07eaa0f4 = -(byte[3]!=0) & m_nTempMyTradeGold — toggle effect bit
//
//   case 0x3b:  DAT_07eaa0f0 = *(puVar8+2)   — 4-byte misc update
//
//   case 0x3c:  PIN/SecondPassword UI control:
//               0 → m_bYourConfirm = 0
//               1 → m_bYourConfirm = 1; UI_SetScene(0x19)
//               2 → m_bMyConfirm = 0; UI_SetScene(0x19)
//               * → UI_SetScene(0x19)
//
//   case 0x3d:  FUN_004337f0(puVar8)
//
//   case 0x40:  DAT_07eaa0e4 = byte[3]*0x100 + byte[2]
//               SetErrorMessage(0x78)   — ShowErrorDialog(0x78)
//
//   case 0x41:  Shop slot display — inner switch byte[3] (0-5):
//               pcVar21/pcVar26 → Widget_Draw(pcVar21, pcVar26, 2)
//               SetErrorMessage(0)
//
//   case 0x42:  FUN_00434660(puVar8)
//
//   case 0x43:  DAT_07eaa0e0 = 0
//               Widget_Draw(&DAT_05826d78, &DAT_07d4e96c, 2)
//
//   case 0x44:  Party/group HP bars:
//               Loop byte[3] count: byte = puVar8[2+n]
//               DAT_07e11e98[(upper nibble)*0x24] = min(lower nibble, 10)
//
//   case 0x45:  FUN_00429c50((float)puVar8)
//   case 0x46:  FUN_00436d60(puVar8)
//
//   case 0x50:  DAT_07eaa0d8 = byte[3]*0x100 + byte[2]
//               SetErrorMessage(0x77)   — ShowErrorDialog(0x77)
//
//   case 0x51:  FUN_00434780(puVar8)
//   case 0x52:  FUN_004348b0(puVar8)
//   case 0x53:  FUN_00434950(puVar8)
//
//   case 0x54:  Second password / PIN full reset:
//               DAT_07eaa114-117 = 0
//               HashTable_Insert_Short(&MAIN_HASH_CLASS, &DAT_07eaa118); DAT_07eaa118=0
//               PACKET_ENCRYPT; DAT_07eaa119=0; DAT_00559f5f=0; DAT_07eaa14c=0
//               HashTable_Insert_Short(&MAIN_HASH_CLASS, &DAT_07eaa11b); DAT_07eaa11b=0
//               PACKET_ENCRYPT; DAT_07eaa124=1; DAT_07eaa144=0
//
//   case 0x55:  Character list change (delete/create result):
//               DAT_07eaa124=1; DAT_07eaa144=1; GuildInputEnable=1
//               DAT_00559c84=0; ClearInput(0)
//               InputTextMax=8; InputNumber=0
//               *(DAT_07abf5d8+0x1da) = 999
//
//   case 0x56:  FUN_00435280(puVar8)
//
//   case 0x5a:  NPC / trade item list:
//               Loop byte[2] count; stride 0x2a per entry
//               FUN_00434dc0(entityId, puVar23, puVar23+2)
//
//   case 0x5b:  FUN_00435110(puVar8)
//
//   case 0x5c:  Entity trade/duel update:
//               entity = FindCharacterIndex(byte[3]*256 + byte[2])
//               iVar20 = FUN_00434dc0(0xffffffff, puVar8+5, puVar8+0xd)
//               entity[0x1da] = (short)iVar20
//               GuildWar_UpdateEntityRelation(entity)
//
//   case 0x5d:  entity = FindCharacterIndex(byte[3]*256 + byte[2])
//               DAT_07eaa114 = 0
//               *(DAT_07abf5d0 + entity*0x394 + 0x1da) = 0xffff
//               DAT_07eaa0d0 = 0xffffffff
//
//   case 0x60:  FUN_004353e0(puVar8)
//   case 0x61:  FUN_00435390(puVar8)
//   case 0x62:  FUN_004354f0(puVar8)
//   case 0x63:  FUN_00435aa0(puVar8)
//
//   case 0x64:  DAT_05826ca4 = byte[3]; DAT_05826ca8 = byte[2]; DAT_05826d30 = 1
//
//   case 0x71:  Party_PacketHandler()
//   case 0x73:  FUN_00433a80(puVar8, iVar20)
//   case 0x81:  FUN_00434170(puVar8)
//   case 0x82:  FUN_00434400()
//   case 0x83:  FUN_00434450(puVar8)   — second password response (0x83 server ack)
//   case 0x86:  FUN_004366c0(puVar8)
//   case 0x87:  FUN_004367d0()
//   case 0x90:  FUN_00436820(puVar8)
//   case 0x91:  FUN_00436cb0(puVar8)
//   case 0x92:  StartMatchCountDown(byte[3] + 1)
//   case 0x93:  FUN_00436a80(puVar8)
//   case 0x94:  FUN_004372c0(puVar8)
//   case 0x95:  FUN_00437380(puVar8)
//   case 0x96:  FUN_004373a0(puVar8)
//   case 0x99:  FUN_004373d0(puVar8)
//   case 0x9a:  FUN_00436ac0(puVar8)
//   case 0x9b:  FUN_00436e40(puVar8)
//   case 0x9c:  FUN_0042e5c0(puVar8, iVar20)
//   case 0x9d:  FUN_00437400(puVar8)
//   case 0xa0:  FUN_00437450(puVar8)
//   case 0xa1:  FUN_00437480(puVar8)
//   case 0xa2:  FUN_004374b0(puVar8)
//   case 0xa3:  FUN_004374e0(puVar8)
//
// ── OPCODE 0xF1 — LOGIN RESPONSE ─────────────────────────────────────────────
//
//   Outer: switch on byte[3] (sub-opcode):
//
//   F1/00: FUN_00424010(puVar8)         — account list
//
//   F1/01: Login result — inner switch on byte[2]:
//     0x00 → DAT_05826cb0=0x15, state=3  (login rejected)
//     0x01 → DAT_05826cb0=0x14, LogIn=2, FUN_00412a70(), state=3
//                                        (login OK, request char list)
//     0x02 → DAT_05826cb0=0x16, state=3  (wrong password)
//     0x03 → DAT_05826cb0=0x17, state=3  (account banned)
//     0x04 → DAT_05826cb0=0x18, state=3
//     0x05 → DAT_05826cb0=0x19, state=3
//     0x06 → DAT_05826cb0=0x1a, state=3; CErrorReport_Write("Version_dismatch")
//     0x07 → DAT_05826cb0=0x1b (default), state=3
//     0x08 → DAT_05826cb0=0x1c, state=3
//     0x09 → DAT_05826cb0=0x25, state=3
//     0x0a → DAT_05826cb0=0x1d, state=3
//     0x0b → DAT_05826cb0=0x1e, state=3
//     0x0c → DAT_05826cb0=0x1f, state=3
//     0x0d → DAT_05826cb0=0x20, state=3
//     0x11 → DAT_05826cb0=0x26, state=3
//     0xc0/0xd0 → DAT_05826cb0=0x22, state=3
//     0xc1/0xd1 → DAT_05826cb0=0x23, state=3
//     0xc2/0xd2 → DAT_05826cb0=0x24, state=3
//
//   F1/02: FUN_004247d0(puVar8, iVar20)  — server list data
//
//   F1/03: Character list result:
//     byte[2]==0 → DAT_05826cb0=0x3f
//     byte[2]==1 → XOR decrypt 30 bytes with PacketXorKey3[i%3]
//                  DAT_05826cb0=0x3e; copy decrypted name → DAT_05826bdc
//
//   F1/04: Version/token result:
//     byte[2]==0 → DAT_05826cb0=0x41
//     byte[2]==1 → XOR decrypt 10 bytes, DAT_05826cb0=0x40, copy → DAT_055ca050
//     byte[2]==2 → DAT_05826cb0=0x42
//     byte[2]==3 → DAT_05826cb0=0x43
//
//   F1/05:
//     byte[2]==0 → DAT_05826cb0=0x45
//     byte[2]==1 → DAT_05826cb0=0x44
//     byte[2]==2 → DAT_05826cb0=0x46
//     byte[2]==3 → DAT_05826cb0=0x47
//
//   F1/12: Login/char-select gate:
//     byte[2]==0 → DAT_05826cb0=0x0c
//     byte[2]==1 → DAT_05826cb0=0x0b   (char select OK)
//     byte[2]==2 → DAT_05826cb0=0x0d
//
// ── OPCODE 0xF3 — CHAR LIST / ENTER WORLD ────────────────────────────────────
//
//   Dispatch on byte[3] (C1) or byte[2] (C2):
//
//   F3/00: FUN_00424240(puVar8)          — receive character list entries
//   F3/01: FUN_00424390(puVar8)          — receive character detail
//   F3/02: byte[2]==1 → DAT_05826cb0=0x39; else → DAT_05826d20=byte[2], 0x3a
//   F3/03: FUN_00425840(puVar8, iVar20)
//   F3/04: FUN_004264d0()
//   F3/05: FUN_00431180()
//   F3/06: FUN_00431480(puVar8)
//   F3/07: EXP update:
//          XOR decode puVar8+2 (3-byte key PacketXorKey3[i%3])
//          STRUCT_DECRYPT → decode g_CharData; g_CharData[0x1c] -= decoded_exp
//          STRUCT_ENCRYPT → re-encode g_CharData
//   F3/08: FUN_00431dc0(puVar8)
//   F3/10: FUN_00426cf0(puVar8, iVar20)
//   F3/11: FUN_004269f0(puVar8)
//   F3/13: entity=FindCharacterIndex(byte[2]*256+byte[5]); ChangeCharacterExt(entity, puVar8+7)
//   F3/14: DAT_07e91388=0; InsertInventoryItem(&DAT_07ea8410,8,8,byte[2],puVar8+5,0)
//          PlayBuffer(0x31, 0, 0)
//   F3/20: DAT_05826d24 = byte[2]
//   F3/22: DAT_05826c08 = puVar8[2]
//   F3/23: Server info block:
//          DAT_05826cc0-cc4 ← puVar8+2 (8B)
//          DAT_05826cc9-ccd ← puVar8+0xd (8B)
//          DAT_05826ca4=byte[6]; DAT_05826ca8=byte[0x15]
//          DAT_05826d33 = (byte[6] != 0xff)
//          DAT_05826cc8 = 0
//   F3/30: FUN_00436fb0()
//   F3/40: FUN_00436550(puVar8)
//
// ── OPCODE 0xF4 — SERVER REDIRECT ────────────────────────────────────────────
//
//   Dispatch on byte[3] (C1) or byte[2] (C2):
//
//   F4/02: FUN_00423e10(puVar8)          — reconecta a otro puerto/IP
//   F4/03: Server redirect:
//          Parse IP from puVar8+2; Net_Disconnect(SocketClient)
//          CreateSocket(ip, port) — connect to new server
//          if result != 0: g_bGameServerConnected = 1
//          crt_sprintf + Widget_Draw — show "connecting" UI
//   F4/05: DAT_05826cb0=1; DAT_083a7c14=1   — back to Connecting state
//
// ── DEFAULT ───────────────────────────────────────────────────────────────────
//
//   Unrecognized opcode → Item_ReturnPickedItem() (log/discard)
//
// ── C2 / ENCRYPTED PACKET PATH ───────────────────────────────────────────────
//
//   if (byte[0] == 0xC2):  puVar9 = byte[1]*256+byte[2]; opcode = byte[3]
//   if (byte[0] == 0xC3):  CSimpleModulus_Decode → decrypt 1-byte-len; goto LAB_00439505
//   if (byte[0] == 0xC4):  CSimpleModulus_Decode → decrypt 2-byte-len; goto LAB_00439505
//   Los dos caminos desencriptados vuelven al mismo LAB_00439505 → switch de opcodes
//
//   Packet sequence tracking (anti-replay / dedup):
//     g_byPacketSerialRecv = rolling sequence counter
//     HashTable_GetIndex / operator_new(2) / HashTable_Insert / HashTable_Remove
//     → trackea los IDs de secuencia de paquetes en vuelo, manda NACK si no coinciden
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_0043E010  → Net_GetFreeBuffer(pool)
//   FindCharacterIndex  → Entity_GetIndex(entityId)  — returns 0-based entity slot
//   DeleteCharacter  → Entity_Spawn(entityId)     — create or update entity slot
//   CreateChat  → Entity_UpdateNameData(name, data, entity, 0, -1)
//   FUN_004801c0  → World_StateInit()          — inicializa el estado in-world después del 0x02
//   FUN_00412de0  → Auth_ProcessChallenge(byte) — handshake response for opcode 0x03
//   FUN_0043bde0  → Entity_SetFlag(flag, entity)
//   FUN_0043c070  → Entity_UpdateFlags(flags, entity)
//   FUN_00500a80  → BufferedPacket_Process()   — processes 0x0b slot buffer
//   FUN_00428210  → FUN_00428210(puVar8, iVar20) — 0x1c handler
//   FUN_00427a00  → PacketHandler_0x0d(puVar8)
//   FUN_00427b90  → PacketHandler_0x10(puVar8)
//   FUN_00427f40  → PacketHandler_0x11(puVar8)
//   FUN_00429690  → PacketHandler_0x12(puVar8, puVar9)
//   FUN_0042a230  → PacketHandler_0x13(puVar8)
//   FUN_0042acc0  → PacketHandler_0x15(puVar8)
//   FUN_0042db60  → PacketHandler_0x16(puVar8, iVar20)
//   FUN_0042f030  → PacketHandler_0x17(puVar8)
//   FUN_0042b4f0  → PacketHandler_0x18(puVar8)
//   Skills_PacketHandler  → PacketHandler_0x19(puVar8, puVar9, iVar20)
//   FUN_0042d780  → PacketHandler_0x1a(puVar8)
//   FUN_0042cd10  → PacketHandler_0x1e(puVar8, puVar9, iVar20)
//   FUN_0042a530  → PacketHandler_0x1f(puVar8)
//   FUN_0042f240  → PacketHandler_0x20(puVar8)
//   FUN_0042f360  → PacketHandler_0x22(puVar8)
//   FUN_0042f690  → PacketHandler_0x23(puVar8)
//   FUN_0042f9a0  → PacketHandler_0x24(puVar8, iVar20)
//   FUN_00429230  → PacketHandler_0x25(puVar8)
//   FUN_00431780  → Equip_HandleResponse()
//   FUN_00431a90  → PacketHandler_0x27()
//   FUN_004cce00  → ItemTable_SetSlot(slot, table, stride)
//   FUN_004321f0  → PacketHandler_0x29(puVar8, iVar20)
//   FUN_00431ea0  → PacketHandler_0x2a(puVar8)
//   FUN_00437f10  → PacketHandler_0x2c(puVar8)
//   FUN_004301b0  → PacketHandler_0x30(puVar8, iVar20)
//   FUN_00427560  → PacketHandler_0x31(puVar8)
//   InsertInventoryItem  → ItemTable_UpdateSlot(table, stride, size, slot, data, flag)
//   STRUCT_DECRYPT  → CharData_Decode(ctx, g_CharData)  — XOR-decode g_CharData
//   CalculateAll  → CharData_RecalcStats(charData)
//   STRUCT_ENCRYPT  → CharData_Encode(ctx, g_CharData)  — XOR-encode g_CharData
//   FUN_004332e0  → PacketHandler_0x37(puVar8)
//   FUN_004337f0  → PacketHandler_0x3d(puVar8)
//   FUN_00434660  → PacketHandler_0x42(puVar8)
//   FUN_00434780  → PacketHandler_0x51(puVar8)
//   FUN_004348b0  → PacketHandler_0x52(puVar8)
//   FUN_00434950  → PacketHandler_0x53(puVar8)
//   HashTable_Insert_Short  → HashTable_RefDecrement(ctx, key)
//   FUN_00435280  → PacketHandler_0x56(puVar8)
//   FUN_00434dc0  → Trade_GetItemData(entityId, data, extraData)
//   FUN_00435110  → PacketHandler_0x5b(puVar8)
//   GuildWar_UpdateEntityRelation  → Entity_UpdateMisc(entity)
//   FUN_004353e0  → PacketHandler_0x60(puVar8)
//   FUN_00435390  → PacketHandler_0x61(puVar8)
//   FUN_004354f0  → PacketHandler_0x62(puVar8)
//   FUN_00435aa0  → PacketHandler_0x63(puVar8)
//   Party_PacketHandler  → PacketHandler_0x71()
//   FUN_00433a80  → PacketHandler_0x73(puVar8, iVar20)
//   FUN_00434170  → PacketHandler_0x81(puVar8)
//   FUN_00434400  → PacketHandler_0x82()
//   FUN_00434450  → SecondPassword_HandleResponse(puVar8)
//   FUN_004366c0  → PacketHandler_0x86(puVar8)
//   FUN_004367d0  → PacketHandler_0x87()
//   FUN_00436820  → PacketHandler_0x90(puVar8)
//   FUN_00436cb0  → PacketHandler_0x91(puVar8)
//   StartMatchCountDown  → CharSelect_SetSlotCount(count)
//   FUN_00436a80  → PacketHandler_0x93(puVar8)
//   FUN_004372c0  → PacketHandler_0x94(puVar8)
//   FUN_00437380  → PacketHandler_0x95(puVar8)
//   FUN_004373a0  → PacketHandler_0x96(puVar8)
//   FUN_004373d0  → PacketHandler_0x99(puVar8)
//   FUN_00436ac0  → PacketHandler_0x9a(puVar8)
//   FUN_00436e40  → PacketHandler_0x9b(puVar8)
//   FUN_0042e5c0  → PacketHandler_0x9c(puVar8, iVar20)
//   FUN_00437400  → PacketHandler_0x9d(puVar8)
//   FUN_00437450  → PacketHandler_0xa0(puVar8)
//   FUN_00437480  → PacketHandler_0xa1(puVar8)
//   FUN_004374b0  → PacketHandler_0xa2(puVar8)
//   FUN_004374e0  → PacketHandler_0xa3(puVar8)
//   FUN_00424010  → Login_RecvAccountList(puVar8)   — F1/00
//   FUN_004247d0  → Login_RecvServerList(puVar8, iVar20) — F1/02
//   FUN_00424240  → CharList_RecvList(puVar8)       — F3/00
//   FUN_00424390  → CharList_RecvDetail(puVar8)     — F3/01
//   FUN_00425840  → CharList_RecvExtra(puVar8, iVar20) — F3/03
//   FUN_004264d0  → CharList_RecvEnd()              — F3/04
//   FUN_00431180  → PacketHandler_F3_05()
//   FUN_00431480  → PacketHandler_F3_06(puVar8)
//   FUN_00431dc0  → PacketHandler_F3_08(puVar8)
//   FUN_00426cf0  → PacketHandler_F3_10(puVar8, iVar20)
//   FUN_004269f0  → PacketHandler_F3_11(puVar8)
//   ChangeCharacterExt  → Entity_SetExtraData(entity, data)
//   FUN_00436fb0  → PacketHandler_F3_30()
//   FUN_00436550  → PacketHandler_F3_40(puVar8)
//   FUN_00423e10  → Net_RecvRedirect(puVar8)        — F4/02
//   CreateSocket  → Net_Connect(ip, port)
//   CSimpleModulus_Decode  → Packet_Decrypt(ctx, outBuf, data, len)  — C3/C4 cipher
//   CSimpleModulus_Encode  → Packet_Encode / CRC_Compute
//   FUN_00412a70  → FUN_00412a70()  — se llama al login OK (F1/01/01)
//   CErrorReport_Write  → Log_SetString(buf, str)
//   PlayBuffer  → UI_SetScene(id, 0, 0)
//   UIChatLogWindow_AddText  → Widget_Draw(element, textureData, flag)
//   SetErrorMessage  → ShowErrorDialog(id)
//   Item_ReturnPickedItem  → Packet_Unknown_Log()
//   PACKET_DECRYPT → HashTable_GetOrInsert
//   PACKET_ENCRYPT  → HashTable_Decrement
//   HashTable_Insert  → HashTable_Insert
//   Packet_DecryptByte  → HashTable_Remove
//   HashTable_GetNode  → HashTable_Get
//   Packet_EncryptByte  → HashTable_Free(entry, key)
//   HashTable_GetIndex → Item_ReturnPickedItem area (addr in binary)
//   FUN_0043de60  → Net_Throttle()
//   Net_Disconnect at 0043dc90
//   operator_new  → MSVC heap alloc
//
// ── FUNCIONES AUXILIARES DE RED / MOVIMIENTO (0x43bde0..0x43ff60) ───────────
//
//   Todas en el mismo rango de dirección que Net_ProcessPacket pero son
//   funciones utilitarias de entidad/movimiento/math que los handlers llaman.
//
// ── NET CONTEXT MANAGEMENT ───────────────────────────────────────────────────
//
//   0x0043daf0  NetCtx_Clear(int ctx)   __fastcall
//     Limpia el buffer del contexto de red:
//       memset(ctx+0x401c, 0, 0x96258*4)   — packet buffer
//       ctx+0x4014 = 0; ctx+0x4018 = 0     — head/tail ptrs
//     Return: ctx
//
//   0x0043db30  Net_WSAInit(int ctx)   __fastcall
//     WSAStartup(0x0202, &local_190)
//     Si error: log "Winsock_DLL_Initialize_error" + MessageBoxA("IError") → return 0
//     Si versión OK (2.2): ctx+8=0; ctx+4=wVersion; CWsctlc__LogPrintOn(); return 1
//
//   0x0043dbf0  Net_CreateSocket(void* this, HWND hWnd)   __thiscall
//     socket(AF_INET=2, SOCK_STREAM=1, IPPROTO_TCP=0) → this+8
//     g_bGameServerConnected = 0  (connected flag)
//     Si INVALID_SOCKET: log error + MessageBoxA → return 0
//     *this = hWnd  (guarda HWND para WSAAsyncSelect)
//     return 1
//
// ── ENTITY ANGLE / MOVEMENT MATH ─────────────────────────────────────────────
//
//   Estas funciones están en el mismo rango de dirección pero son math de
//   movimiento de entidades. Se documentan acá porque son llamadas por los
//   handlers de movimiento en Net_ProcessPacket.
//
//   0x0043e050  Entity_GetDirCode(float x1,y1, float x2,y2) → ushort
//     dx = x2-x1; dy = y2-y1
//     Math_Fabs(dx) = abs o sqrt
//     Si |dx| < _DAT_00552868: retorna código de dirección vertical (N/S)
//     Else: calcula atan2 → código de dirección ushort (8 direcciones)
//
//   0x0043e120  Angle_ShortestPath(int cur, int target, int maxStep) → int
//     Calcula la diferencia más corta entre ángulos (wraparound a 0x168=360)
//     Si |delta| > 180: ajusta vía offset de 360
//     Retorna min(|delta|, maxStep) con signo
//
//   0x0043e1b0  Angle_Interpolate(float cur, float target, float step) → float10
//     Normaliza ambos ángulos (+ 360 si < 0)
//     Interpola suavemente target → cur respetando wraparound de 360
//
//   0x0043e370  Angle_Delta(float from, float to, char wrap) → float10
//     Normaliza ambos; retorna (to - from) con wraparound de ±180
//
//   0x0043e430  Math_Atan2_ToAngle(float x1,y1, float x2,y2) → int
//     fpatan((y2-y1)/(x2-x1)) → ángulo en grados enteros
//     Si x2 < x1: ajusta 180°. Resultado en [0..360)
//
//   0x0043e4a0  Entity_UpdateFacing(float* pos, float* entity, float* target, float step)
//     Llama Entity_GetDirCode(pos, target) → código dirección
//     Llama Angle_Interpolate(entity[2], dirCode, step) → entity[2] (ángulo)
//     dx=pos-target; distancia 3D; actualiza ángulo de elevación
//
//   0x0043e570  Entity_ApplyRotation(float* out, float* quat, float* vec)
//     Matrix_BuildFromEuler(quat, mat4x3) — quaternion → matriz rotación
//     Vector_Rotate(vec, mat4x3, &local) — matriz × vector
//     out[0..2] += local[0..2]  (aplica rotación al offset)
//
//   0x0043e5c0  Entity_SmoothAngle(int entity)
//     entity+0x161 (char flag): si 0 → lerp suave hacia target:
//       entity+0x168 += (entity+0x164 - entity+0x168) * _DAT_005524f4
//     Si flag != 0 → snap directo (o animación invertida)
//
//   0x0043e680  Entity_UpdatePath(int p1,p2,p3,p4)
//     Función compleja (~40 líneas) con muchos floats y ftol
//     Actualiza la trayectoria de movimiento de una entidad
//
//   0x0043e820  Entity_SetAnimation(int entity, uint animId)
//     Verifica animId < animData[entity.type * 0xbc + 0x26] o == 0x4c/0x4d
//     Si animId != anim actual:
//       entity+0x106 = entity+0x105  (prev anim)
//       entity+0x10c = entity+0x108  (prev anim timer)
//       entity+0x105 = animId; entity+0x108 = 0  (reset timer)
//
//   0x0043e890  Entity_FaceTarget(int entity, int target)
//     Si target != 0:
//       Entity_GetDirCode(entity.pos, target.pos)
//       Angle_Delta(entity.angle, dirCode, 1)
//       Actualiza entity.angle suavemente
//
//   0x0043e940  Entity_AnimTick(int entity)
//     Lee entity+0x105 (anim state). Si != 0x06: avanza frame de animación
//     Máquina de estados de animación (idle/walk/attack/die/...)
//
//   0x0043ea20  Angle_ToDir(float angle, char mode) → undefined4
//     Convierte ángulo float a código de dirección / byte de dirección
//     Considera parámetro mode para inversión o modo especial
//
// ── PACKET QUEUE / ACTION QUEUE ──────────────────────────────────────────────
//
//   0x0043f2d0  PacketBuf_Free(void)
//     Libera buffers en DAT_05826df4 (índices 0xff y 0x102)
//     operator_delete para cada buffer no-NULL
//
//   0x0043f3e0  PacketQueue_Enqueue(uint id, float param2, uint p3, uint p4, void* data, float p6)
//     Wrapper: llama PATH_FindPath(DAT_05826df4, id, param2, p3, p4, 1, 2, p6)
//     local_4 = 2 (prioridad/tipo)
//
//   0x0043f500  ActionQueue_Insert(void* this, int id, float t, int p3, int p4, int p5, int p6, float p7)
//     __thiscall; inserta una acción en la cola de acciones del servidor
//     Gestiona slots, timestamps, prioridades
//     Usado para movimiento suavizado y predicción del cliente
//
//   0x0043fd30  Queue_SetRange(void* this, int param_1)   __thiscall
//     Clamp param_1 a [0, this+8)
//     Actualiza this+0x400 (puntero de escritura) y this+0x404 (puntero de lectura)
//     → control de rango circular del buffer
//
//   0x0043fd70  AnimTimer_Update(void)
//     Si FpsTimerInitialized == 0: init (FpsWindowStartTimeMs=timeGetTime(), FpsTimerInitialized=1)
//     Si no: actualiza DAT_05826e10 (delta del timer de frame) vía timeGetTime()
//     → timer global de animación, usado por Entity_AnimTick
//
//   0x0043fea0  LinkedList_Add(void* this, undefined4 data, int param_2)   __thiscall
//     Si this+8 == 0: this+4 += 1; operator_new(0x14); enlaza nodo
//     Linked list de nodos 0x14 bytes (data + next ptr)
//
//   0x0043ff60  LinkedList_Remove(undefined4* param_1)   __fastcall
//     Traversal de la lista enlazada; desenlaza y libera nodo
//     Usado por PacketQueue_Enqueue para gestión de memoria
//
// ── LOGIN / SEND BUILDERS ─────────────────────────────────────────────────────
//
//   0x0043c250  Login_BuildEncPacket(byte p1, byte p2, byte p3, byte p4)   __cdecl
//     Construye paquete 0xC1/0xF1 con buffer local_d3c[32] (32-byte XOR key)
//     Idéntico al bloque XOR de Net_ProcessPacket (caso 0x03/0x36)
//     Usado para re-auth desde Scene_Dispatch
//
//   0x0043ce50  CharSelect_BuildPacket(byte sub, undefined4 data)   __cdecl
//     Construye paquete de char-select con buffer local_434[32]
//     Envía selección de personaje con XOR encrypt
//
// ── ENTITY FLAGS (referenciados en opcodes) ───────────────────────────────────
//
//   0x0043bde0  Entity_SetFlag(byte flag, entity*)
//     Activa el flag especificado en entity->flags
//
//   0x0043c070  Entity_UpdateFlags(uint flags, entity*)
//     Actualiza múltiples flags de la entidad de una vez
//
//   0x0043c250  → ver Login_BuildEncPacket arriba
//
//   0x0043d3e0  HashTable_Insert2(void* this, undefined4)   __thiscall
//     Inserta en HashTable con lógica de colisión (abStack_14[4] key)
//
//   0x0043d670  HashTable_Find2(void* this, undefined4*)   __thiscall
//     Busca en HashTable, retorna undefined4 (ptr o índice)
```

### Línea 660 — antes de `// ============================================================================`

```cpp
// 2026-05-04: Hero equipment stash (definidos en Render_PlayerEquipment.cpp).
// F3/03 los popula; HeroEquipWatchdog los re-aplica per-frame.
```

### Línea 663 — antes de `extern int __fastcall CWsctlc_GetReadMsg(int poolBase);   // GetReadMsg (stubs.cpp) — retu`

```cpp
//
// Esta primera iteración implementa el scaffolding completo + los opcodes del
// flujo de login (F1/00, F1/01, F4/02, F4/03, F4/05). Los handlers opcode
// complejos (combate, movimiento, char list, etc.) son stubs que registran
// llegada pero no avanzan estado — se añaden cuando el server los dispara.
//
// Paquetes cifrados (C3/C4) todavía no se desencriptan aquí: en el flujo
// inicial ConnectServer → cliente solo envía/recibe C1/C2.
```

### Línea 685 en `CWsctlc_GetReadMsg`

```cpp
// GetReadMsg (stubs.cpp) — returns ptr as int
```

### Línea 695 — antes de `extern "C" BYTE OffsetInventoryItems[];`

```cpp
// 2026-05-08: inventory/warehouse pool symbols (defined in HUD_Pass3.cpp).
```

### Línea 703 en `Entity_FindById`

```cpp
// stubs.cpp
```

### Línea 712 — antes de `static void ShopInsertItem(int slot, const BYTE* Item)`

```cpp
// ── ShopInsertItem (PORT FIEL de IDA sub_4CC0E0, 2026-07-25) ─────────────────
// Inserta un item de tienda en el pool Inventory[32 + slot], llenando su
// footprint Width×Height (de ItemAttribute[type]).  El item de tienda son 4
// bytes: [typeLo][levelByte][durability][flags].  Key se setea solo en la celda
// primaria (el render usa Key>0 como gate).  El ItemConvert completo (que llena
// DamageMin/Defense/etc para el tooltip) se omite: el tooltip los recalcula
// on-hover desde ItemAttribute[type].  Pool shop = Inventory[32..151] (grid 8×15).
```

### Línea 733 en `ShopInsertItem` — antes de `BYTE* cell = ShopItems + idx * 0x44;`

```cpp
            // CRÍTICO: el pool de tienda es un overlay que arranca en
            // &Inventory[idx].WalkSpeed (offset +24), NO en el base del ITEM.
            // El render (sub_4E38B0, HUD_Pass3:382) recibe &Inventory[32].WalkSpeed
            // y lee Type@+0, Level@+4, Durability@+26, Option1@+27, Key@+0x38
            // relativo a ese puntero. sub_4CC0E0 escribe con el mismo convenio.
```

### Línea 744 en `ShopInsertItem` — antes de `cell[62]               = (BYTE)(slot % 8);             // x`

```cpp
            // CRÍTICO (2026-07-27): x/y = posición-origen del item en el grid
            // (slot%8, slot/8), escrito en TODAS las celdas del footprint (igual
            // que sub_4CC0E0 ->x=a1%8 ->y=a1/8). El hover (Item_ClickHandler:634)
            // normaliza celdas de footprint al origen vía `inv_base + 34*(8*y+x)`.
            // Sin esto x/y=0 → TODA celda normaliza a slot 0 → el tooltip siempre
            // mostraba el primer item sin importar cuál hovereabas.
```

### Línea 807 en `ItemMove_RestoreSlot` — antes de `BYTE wire[6] = { 0, 0, 0, 0, 0, 0 };`

```cpp
        // 2026-07-27 FIX "el item se transforma en otro al moverlo":
        // item68 es el ITEM struct de 68 bytes copiado del slot al hacer pickup,
        // NO formato wire. InsertInventoryItem/InsertInventoryItem espera 4-5 bytes wire
        // [typeLo][optByte][dur][hi][ext]; pasarle el struct crudo reinterpretaba
        // Type-high/Level-int/etc como opciones → el item restaurado quedaba con
        // type/opciones equivocadas. Se disparaba en CADA move denegado (server
        // devuelve result=FF cuando el inventario está lleno / slot destino
        // ocupado) → el item de origen se corrompía. Reconstruimos el wire desde
        // los offsets conocidos de la struct (Type@0, Level@4, Durability@26,
        // Unkown@60=byteHi, byColorState@61=ext).
```

### Línea 1265 — antes de `static void NetLog(const char* fmt, ...);   // definida mas abajo`

```cpp
// ── Teclas de skill del F3/30 ───────────────────────────────────────────────
// El paquete trae `tecla -> tipo de skill` y el cliente guarda lo contrario
// (`slot -> tecla`, 64 bytes por personaje en CharacterAttribute+215), asi que
// para traducirlo hay que buscar cada skill en la lista del personaje
// (CharacterAttribute+87), que la puebla el F3/11.
//
// 2026-09-25: MuEmu manda el F3/30 ANTES del F3/11 (verificado en debug.log:
// Option llega ~20 paquetes antes que SkillList), asi que al traducir la lista
// todavia estaba vacia, ninguna skill matcheaba y el mapa quedaba entero en
// 0xFF -- las teclas asignadas se perdian en cada login por mas veces que se
// reasignaran.  Se guardan los 10 bytes y se aplica el mapeo dos veces: al
// recibir el F3/30 (por si la lista ya estuviera, que es el orden que asume
// IDA) y de nuevo al final del snapshot del F3/11.
```

### Línea 1303 — antes de `static void Recv_NewCharacterCalc(const BYTE* Msg, int Size)`

```cpp
//
// 2026-09-21 (issue #54, "defensa rate y dano se cruzan al subir de nivel"):
// el port asumia que despues de MagicSpeed venian directo MagicDmgMin/Max y
// leia AttackSuccessRate en p+40, Defense en p+48 y DefenseSuccessRate en p+52.
// Faltaban los 4 campos del medio, asi que cargaba:
//    tasa de ataque   <- MagicDmgMin        (en la captura: 3678)
//    defensa          <- MagicDmgRate       (53)
//    tasa de defensa  <- AttackSuccessRate  (42652)
// Los tres numeros de la captura cuadran exactos.  Antes de subir de nivel se
// veian bien porque venian del recalculo local (CalculateAll); el server manda
// el E1 al subir, y ahi se pisaban.
```

### Línea 1357 en `Recv_NewCharacterCalc` — antes de `*(WORD*)(CA + 0x46) = ClampToWord(ViewMagicDamageMin);`

```cpp
    // 2026-09-24: los dos unicos campos que el DLL escribe y este port no
    // (GCNewCharacterCalcRecv, Protocol.cpp:925).  El dano FISICO no viaja
    // por aca -- el DLL tampoco lo escribe, lo sigue calculando el cliente.
```

### Línea 1498 en `Recv_JoinServer` — antes de `HeroKey   = g_HeroKey;`

```cpp
        // FIX 2026-07-24: HeroKey (HeroKey que usa ClearCharacters vía
        // OpenWorld) NUNCA se seteaba → quedaba en 0.  Con eso, al entrar al
        // mundo ClearCharacters(0) conservaba las entidades con Key==0 (incluida
        // la del Hero stale del slot 0 que quedaba de antes del join) → fantasma
        // renderizado + hover pegado.  Ahora lleva el HeroKey real.
```

### Línea 1508 en `Recv_JoinServer` — antes de `HWID_Send();`

```cpp
        // ── 2026-04-25: solo F1/05 HWID re-activado ────────────────────────
        // Tras agregar el LoginKey chain XOR al F1/01 build, el server pasó
        // de mudo a responder con code=05 (HardwareID rechazado / blacklist).
        // El server MuEmu requiere F1/05 SetHwid antes del F1/01 — sin él,
        // CheckHardwareID en Blacklist.cpp considera el HWID vacío como
        // blacklisted y devuelve code 05.
        //
        // Compatibilidad MuEmu: el deploy actual corta la sesión temprano si
        // tras F1/00 no recibe también F1/04 antes de F1/05/F1/01. Esto no
        // existe en el flujo Webzen original; queda aislado acá.
        //   Lang_Send(1);
```

### Línea 1556 en `Recv_LoginResult` — antes de `NetLog("NET:  → F1/01 LOGIN-RESULT code=0x%02X -> state=%u (t=%lu)",`

```cpp
    // 2026-07-25 (#1): log del resultado de login crudo por intento, para
    // diagnosticar el "primer enter = dato mal, segundo enter entra".  Si el
    // primer intento trae un code de fallo (0x02 pass, 0x0C/0x0D, etc.) y el
    // segundo (mismas credenciales) trae 0x01 OK, el problema es el PRIMER
    // paquete (serial/encriptación) o un estado stale; si ambos códigos son
    // iguales, es del server.  Msg[4]=code server, state=nuestro DAT_05826cb0.
```

### Línea 1565 en `Recv_LoginResult` — antes de `{`

```cpp
    // 2026-07-25 (#1): auto-reintento del PRIMER login fallido con code 0x02.
    // El diagnóstico probó que el 1er F1/01 se rechaza (0x02 = pass incorrecta)
    // con datos IDÉNTICOS (userLen/passLen/crc iguales) al 2do intento que SÍ
    // entra (0x01) — quirk del primer paquete C3 contra el server MuEmu, no es
    // la contraseña.  Reintentamos UNA sola vez simulando Enter (DAT_055ca038),
    // que hace que Game_SceneUpdate re-envíe las MISMAS credenciales.  Si la
    // pass fuera realmente incorrecta, el 2do intento también da 0x02 y ahí sí
    // se muestra el error.  Sólo 0x02 — no reintentamos banned/already-online/
    // version-mismatch para no enmascararlos.
```

### Línea 1793 en `Recv_JoinMapServer` — antes de `(void)bEncrypted;`

```cpp
    // BUG-FIX 2026-04-28: el F3/03 que envía el server MuEmu (Protocol.cpp
    // GDCharacterInfoSend → DataServer → DGCharacterInfoRecv → cliente) llega
    // como packet plano C1 (no C3-encriptado vía SimpleModulus). Nuestro
    // dispatcher en Net_ProcessPacket inicializa bEncrypted=false y nunca lo
    // setea. Antes hacía early-return si !bEncrypted asumiendo que era el
    // path "GameGuard re-auth"; pero en MuEmu el path bEncrypted=false ES el
    // path real → skip → hero nunca se posiciona → pantalla negra.
    //
    // El IDA original 0.97K diferenciaba ambos paths para soportar
    // re-handshakes de GameGuard. Nuestro server MuEmu no usa GG, así que
    // ejecutamos siempre el world-load path.
```

### Línea 1874 en `Recv_JoinMapServer` — antes de `if (DAT_07cf1ffc != 0) {`

```cpp
        // 2026-07-27 FIX zen al login: PMSG_CHARACTER_INFO_SEND (F3/03) trae
        // Money (DWORD) en offset 40 — la struct NO es pack(1): tras MaxBP
        // (WORD@36) hay 2 bytes de padding porque Money (DWORD) se alinea a 4 →
        // offset 40. Verificado por hex del paquete: RB[40..43]=D4 17 F1 1B =
        // 0x1BF117D4 (leyendo en 38 daba 0x17D40000 = valor corrido). El zen se
        // muestra desde CharacterMachine+1352 (= DAT_07cf1ffc+1352).
```

### Línea 1886 en `Recv_JoinMapServer` — antes de `if (DAT_07abf5d8) {`

```cpp
        // 2026-07-27 FIX (alas/cuerpo rojos "PK"): PMSG_CHARACTER_INFO_SEND trae
        // PKLevel (BYTE) justo después de Money → offset 44. El render aplica el
        // tinte rojo (1.0,0.1,0.1) cuando entity+0x2EA >= 6
        // (Entity_UpdateRender:333). Entity_Spawn inicializa ese campo en 3 para
        // los mobs, pero el HÉROE no pasa por ese path → quedaba con basura
        // (el diag mostró 2ea=255 → rojo permanente). Ahora guardamos el PKLevel
        // real que manda el server.
```

### Línea 1919 en `Recv_JoinMapServer` — antes de `World = world;`

```cpp
    // BUG-FIX 2026-04-28: leer class + body-part slots del char-select entity
    // ANTES de OpenWorld (que llama ClearCharacters y borra los entities).
    // Body parts (helm/armor/pant/glove/boot) son lo que efectivamente renderiza
    // el cuerpo del hero — sin esto RenderCharacter entra pero no dibuja nada.
```

### Línea 1926 en `Recv_JoinMapServer` — antes de `if (DAT_07abf5d0) {`

```cpp
    // 2026-05-07: WIPE entity pool de slots stale del CharSelect ANTES de
    // OpenWorld + hero spawn. Sin esto, los slots de chars del CharSelect
    // quedan activos con sus nombres en +0x1C1 y aparecen como "NPCs" cuando
    // el hover detect los recoge.
    //
    // Wipea TODOS los slots — el nuevo hero se crea via CreateCharacterPointer
    // a unas líneas más abajo (paso 4-5) en un slot random, sobrescribiendo
    // sea cual sea. Los mobs/players del world via 0x12/0x13 viewport packets
    // llegan DESPUÉS del OpenWorld y populan el pool limpio.
    //
    // Nota: el wipe es FULL (slot[0]=0 + 0x84=0 + 0x160=0 + 0x1C1=0). Si
    // omitía slots por preservar el viejo hero, ese slot quedaba con nombre
    // y kind viejos → seguía apareciendo como entidad fantasma in-world.
    // User reportó "leo nombres del select character" + "entra con walking
    // animation sin mobs" 2026-05-07.
```

### Línea 1950 en `Recv_JoinMapServer` — antes de `{`

```cpp
    // BUG-FIX 2026-04-29: OpenWorld bloquea ~2 segundos cargando
    // BMDs. Durante ese tiempo el server manda ~3KB de packets post-JoinMapServer,
    // pero como nuestro message pump está bloqueado no hacemos recv → server's
    // IoSideBuffer overflows o WSASend falla con WSAENOBUFS → CloseClient.
    // Pumpear la message queue ANTES de empezar el BMD load (drena lo que
    // ya llegó), y al final del load (drena lo nuevo) ayuda a que el server
    // no cierre por backpressure.
```

### Línea 1967 en `Recv_JoinMapServer` — antes de `++g_WorldLoading;`

```cpp
    // 2026-09-02 (monstruos que "cargan mal" al entrar a un mapa): OpenWorld
    // tarda ~2 s cargando BMDs y, para que el server no cierre por backpressure,
    // AccessModel pumpea la cola de mensajes cada 8 modelos.  Ese pump entrega
    // WM_USER -> Net_Recv -> **Net_ProcessPacket**, o sea los handlers corren
    // RE-ENTRANTES en mitad de la carga: el `0x13 ViewportMonster` creaba
    // monstruos cuyo modelo todavia no estaba abierto (visto en debug.log: el
    // spawn del slot 0 cae entre Object01.bmd y Object42.bmd).  De ahi que
    // salieran mal y que alejarse y volver -- que los re-crea con el modelo ya
    // cargado -- los arreglara.
    //
    // `g_WorldLoading` deja que el pump siga DRENANDO el socket (que es lo que
    // evita el backpressure) pero suspende el dispatch: los paquetes quedan en la
    // cola y se procesan al terminar la carga.
```

### Línea 2000 en `Recv_JoinMapServer` — antes de `heroPtr[0x306] = PosX;`

```cpp
    // BUG-FIX 2026-04-28: CreateCharacterPointer setea cached_wp (+0x388/0x38c) pero NO
    // target_grid (+0x306/0x307). El per-frame walker Entity_AdvancePath en
    // Player_InputTick lee target_grid → como inicialmente está en 0,0, el
    // hero camina automáticamente a la esquina del mapa.
    // Forzar target == position para que el walker quede idle hasta el primer click.
```

### Línea 2011 en `Recv_JoinMapServer` — antes de `heroPtr[0x105] = 1;             // anim_state = idle`

```cpp
    // 2026-05-07: post-wipe init de anim_state — sin esto, el wipe deja 0x105=0
    // y anim 0 puede no ser "idle" para algunos models. anim_state=1 es el
    // idle1 standard.
```

### Línea 2023 en `Recv_JoinMapServer` — antes de `*(unsigned int*)(heroPtr + 0xe8) = 0x3e99999a; // 0.3f`

```cpp
    // 2026-06-16: el ReceiveJoinMapServer de IDA NO espeja el cuerpo/equipo desde
    // las entidades del char-select. Asocia al Hero, copia clase/flags, y después reconstruye
    // desde CharacterMachine vía SetCharacterClass(). Acá dejamos sólo la
    // siembra de luz, porque CreateCharacterPointer deja Light en 0.
```

### Línea 2075 en `Recv_JoinMapServer` — antes de `// (11) Stop dungeon BGM 110 si no estamos en mapa-evento (11..16).`

```cpp
    // BUG-FIX 2026-04-29: enviar F3/12 CharacterMoveViewportEnable + 0E LiveClient
    // inmediatamente. El server MuEmu (Protocol.cpp:1439 CGCharacterMoveViewportEnableRecv)
    // pone RegenOk=2 al recibir esto. Sin un ACK del cliente post-JoinMapServer
    // el server cree que el cliente está congelado y cierra el socket en
    // ~50-100ms (visto en logs). El IDA original manda F3/12 al final de su
    // ReceiveJoinMapServer; nuestro port no lo hacía.
    // 2026-04-29: post-F3/03 sends removidos. El keepalive 1Hz en Game_MainLoop
    // manda 0x0E. F3/12 fue la causa probable del kick previo (server-side
    // procesa MainCheck antes de F3/12, y nuestro F3/12 desincronizaba algo).
    //
    // 2026-05-04: send INMEDIATO de keepalive 0x0E al recibir F3/03 — el
    // server espera saber que el cliente sigue vivo apenas recibe el world
    // entry. El keepalive 1Hz en Game_MainLoop puede tardar hasta 1s en
    // arrancar (state==5 transición + frame interval), tiempo que el server
    // a veces no perdona.
    // 2026-05-05: el binario original 0.97k (FUN_00425840 Recv_JoinMapServer)
    // en el path bEncrypted=TRUE (= C3 packet, como manda MuEmu) NO envía
    // NINGÚN packet post-F3/03. Solo procesa los datos del char y carga el
    // mundo. El send de F3/12 ViewportEnable solo ocurre en el path
    // bEncrypted=FALSE (raro, no aplica con MuEmu C3).
    //
    // Sends previos (0x0E keepalive + F3/12 ViewportEnable) eran ADD-ON
    // nuestros que NO existen en el original. El keepalive 1Hz en
    // Game_MainLoop ya cubre el liveness check post-F3/03.
    //
    // RegenOk (=2 normalmente seteado por F3/12) — en MuEmu el server-tick
    // lo procesa y eventualmente lo lleva a 0 (OBJECT_PLAYING) sin necesidad
    // del F3/12 desde cliente.
```

### Línea 2114 — antes de `static void Recv_Revival(const BYTE* Msg, int Size)`

```cpp
// ---------------------------------------------------------------------------
// F3/04 — ReceiveRevival  (@ 0x004264D0)
//
// Respawn tras la muerte.  El server MuEmu lo manda SOLO, por timer: al morir
// pone `DieRegen = 1` (ObjectManager.cpp:2759 / Monster.cpp), y el tick
// `ObjectSetStateCreate` (ObjectManager.cpp:80) lo pasa a 2 cuando venció
// `MaxRegenTime + 1000`; ahí `CObjectManager::Run` (ObjectManager.cpp:262-341)
// restaura Life/Mana/BP, reubica al pj (`CharacterGetRespawnLocation`) y llama
// `GCCharacterRegenSend`.  El cliente NO pide nada: sólo tiene que procesar
// este paquete.  Como no teníamos el handler, el pj quedaba muerto para
// siempre (issue #5).
//
// PMSG_CHARACTER_REGEN_SEND (Protocol.h:426, `setE` → frame C3), con el
// padding de MSVC:
//    +0..3   PSBMSG_HEAD (C3, size, F3, 04)
//    +4      BYTE  X
//    +5      BYTE  Y
//    +6      BYTE  Map
//    +7      BYTE  Dir
//    +8      WORD  Life
//    +10     WORD  Mana
//    +12     WORD  BP
//    +14,15  padding (Experience DWORD se alinea a 4)
//    +16     DWORD Experience
//    +20     DWORD Money
//    +24     DWORD ViewCurHP   (GAMESERVER_EXTRA)
//    +28     DWORD ViewCurMP
//    +32     DWORD ViewCurBP
// Los offsets +8/+10/+12/+16/+20 coinciden exactamente con los que lee IDA
// (`*((_WORD *)ReceiveBuffer + 4/5/6)`, `*((_DWORD *)ReceiveBuffer + 4/5)`).
//
// El ruido de hash-table (STRUCT_DECRYPT/ENCRYPT sobre CharacterMachine, ~60%
// del decompile) se omite por policy del proyecto.
// ---------------------------------------------------------------------------
```

### Línea 2241 en `Recv_Revival` — antes de `++g_WorldLoading;`

```cpp
    // 2026-09-02 (monstruos que "cargan mal" al entrar a un mapa): OpenWorld
    // tarda ~2 s cargando BMDs y, para que el server no cierre por backpressure,
    // AccessModel pumpea la cola de mensajes cada 8 modelos.  Ese pump entrega
    // WM_USER -> Net_Recv -> **Net_ProcessPacket**, o sea los handlers corren
    // RE-ENTRANTES en mitad de la carga: el `0x13 ViewportMonster` creaba
    // monstruos cuyo modelo todavia no estaba abierto (visto en debug.log: el
    // spawn del slot 0 cae entre Object01.bmd y Object42.bmd).  De ahi que
    // salieran mal y que alejarse y volver -- que los re-crea con el modelo ya
    // cargado -- los arreglara.
    //
    // `g_WorldLoading` deja que el pump siga DRENANDO el socket (que es lo que
    // evita el backpressure) pero suspende el dispatch: los paquetes quedan en la
    // cola y se procesan al terminar la carga.
```

### Línea 2551 en `Recv_LogOut` — antes de `CharSelectSceneInitialized = 0;`

```cpp
        // 2026-05-05: RESET scene-init guards. EnterWorldTick (state=4) and
        // CharSelectTick (state=5) tienen guardas de init (IDA: DAT_083a7c4b/4c) que
        // sólo permite re-inicializar la escena la PRIMERA vez. Sin reset,
        // post-JoinChar el substate DAT_083a7c14 quedaba en 0x1c (post-OK-
        // click) heredado del primer login → la siguiente tick disparaba
        // F3/03 select-char inmediato → server respondía con JoinMapServer
        // → cliente "recargaba el mapa" en vez de mostrar char-select.
```

### Línea 2574 en `Recv_LogOut` — antes de `DAT_05826cb0 = 0;                // CurrentProtocolState`

```cpp
        // ── BUG-FIX 2026-08-17: faltaba la cola de ReceiveLogOut ──────────────
        // IDA 0x4247D0 LABEL_117: DESPUÉS del send, la rama sub==1 hace
        // `CurrentProtocolState = 0` e `InitGame()`, igual que la rama sub==2.
        // Sin el InitGame quedaba `World` (World) con el mapa anterior.
        // Eso importa porque nuestro RequestTerrainHeight (Terrain_Utils.cpp:49)
        // gatea con `World < 0` en vez del `SceneFlag != 5` del original — una
        // desviación deliberada por el orden del JoinMapServer. Con World=7
        // (Atlans) heredado y su heightmap todavía cargado, CreateCharacterPointer
        // le daba a cada personaje del char-select la altura del terreno de
        // Atlans en vez de 0 → aparecían flotando más arriba. `World = -1` de
        // InitGame es justamente lo que hace que el guard relajado se comporte
        // como el original acá.
```

### Línea 2786 en `ReceiveTradeExit97k` — antes de `DAT_07eaa11b = 0;`

```cpp
    // AUDITORIA 2026-07-20: aca habia un `else if (state == 4)` con
    // GlobalText[2108] — indice FUERA del Text.bmd del 0.97k (1000
    // filas).  ReceiveTradeExit (IDA 0x4337F0) solo maneja los
    // estados 0, 2 y 3; el 4 es un graft de version posterior.
```

### Línea 2833 en `Net_ProcessPacket` — antes de `BYTE __pktCopy[0x2100];`

```cpp
        // 2026-09-03 -- COPIA DEL PAQUETE ANTES DE PROCESARLO.
        //
        // `CWsctlc_GetReadMsg` (GetReadMsg) devuelve un puntero DENTRO del buffer de
        // recepcion del socket, que es compartido.  Handlers que tardan --
        // sobre todo los de viewport, porque `CreateMonster` carga el BMD del
        // monstruo -- dejan que se bombee la cola de mensajes en el medio, entra
        // un `Net_Recv` y el buffer se sobreescribe MIENTRAS el handler todavia
        // esta recorriendo sus entradas.
        //
        // Medido (sonda MONDBG, 2026-09-03): un `0x13 ViewportMonster count=22`
        // parseo bien su entrada 0 (`id=89 type=253 pos=(207,75)`) y a partir de
        // la 1 empezo a leer el payload de un `F3/E2` que habia llegado en el
        // medio -- de ahi monstruos con ids de ~25700 en pos (0,2), (3,2), (7,2),
        // entre ellos el "Giant" (type 7) inmatable en un mapa sin spawn.
        //
        // Los C3/C4 ya eran inmunes porque se desencriptan a `scratch`; los C1/C2
        // se usaban directo.  Ahora todos se copian.
```

### Línea 2901 en `Net_ProcessPacket` — antes de `plain[1] = (BYTE)(bodyLen + 2);`

```cpp
            // 2026-08-25 FIX (issue #13): `plain[1]` es UN byte, asi que para un paquete
            // de mas de 255 el tamaño se trunca. Medido con el F3/10 del inventario:
            // 306 bytes reales reportaban Size=49 (= 306 & 0xFF).
            //
            // No rompia el inventario porque ese parser itera por `count` y los DATOS
            // del buffer si estan completos — solo el byte de tamaño se pierde. Pero
            // cualquier handler que valide o recorra por `Size` cortaba mal.
            //
            // El arreglo es llevar el tamaño APARTE del buffer: `Size` es un int, se
            // toma del valor real y NO se relee de `Msg[1]`. El byte de `plain[1]`
            // queda truncado igual que antes, pero ya no lo consume nadie.
            //
            // Se descarto re-enmarcar como C2 (que es lo que haria el original para un
            // paquete largo): eso correria +1 todos los offsets del cuerpo y obligaria
            // a revisar cada handler que hoy asume el layout C1. Con el tamaño aparte
            // el layout no cambia y el fix es cerrado.
```

### Línea 2923 en `Net_ProcessPacket` — antes de `if (Size > 0xFF) {`

```cpp
            // El byte truncado se loguea al lado a proposito: cuando difiere de
            // `Size`, esa linea es un paquete que ANTES se procesaba con el
            // tamaño equivocado (ver el fix del issue #13).
```

### Línea 2993 en `Net_ProcessPacket` — antes de `Recv_Revival(Msg, Size);`

```cpp
                        // Respawn tras la muerte (issue #5).  Ver Recv_Revival.
```

### Línea 3001 en `Net_ProcessPacket` — antes de `if (Size < 10 || !CharacterAttribute) {`

```cpp
                        // ── F3/06 PMSG_LEVEL_UP_POINT_SEND ───────────────────
                        // 2026-08-08 FIX ("al subir un punto los números se
                        // vuelven locos"): el port leía un layout INVENTADO
                        // (`WORD` en Msg+5 y Msg+7). El struct real del server
                        // (Protocol.h:467, GAMESERVER_EXTRA=1 en stdafx.h:8) es,
                        // con el padding de MSVC:
                        //    +0..3  PSBMSG_HEAD  (C1, size, F3, 06)
                        //    +4     BYTE result  (= 16 + type, 0 = rechazado)
                        //    +5     padding                     ← el port leía acá
                        //    +6     WORD MaxLifeAndMana
                        //    +8     WORD MaxBP
                        //    +10    padding (alineación a 4)
                        //    +12    DWORD ViewPoint      (LevelUpPoint restante)
                        //    +16    DWORD ViewMaxHP
                        //    +20    DWORD ViewMaxMP
                        //    +24    DWORD ViewMaxBP
                        //    +28    DWORD ViewStrength
                        //    +32    DWORD ViewDexterity
                        //    +36    DWORD ViewVitality
                        //    +40    DWORD ViewEnergy
                        //    sizeof = 44
                        // Los campos View* son autoritativos (el server manda el
                        // estado COMPLETO), así que no hace falta decrementar
                        // LevelUpPoint ni incrementar el stat a mano.
                        // No hay nada que desencriptar: el server usa
                        // `header.set` (no `setE`), o sea C1 plano.
```

### Línea 3276 en `Net_ProcessPacket` — antes de `if (Size < 5) {`

```cpp
                        // 2026-05-06: port FIEL desde server source
                        // Mu-linux-97K/Source/MuServer/GameServer/SkillManager.cpp:2256
                        // GCSkillListSend. Wire format:
                        //   [C1][size][F3][11][count] [slot][skill][level]×count
                        //
                        // Per PMSG_SKILL_LIST (server SkillManager.h:161):
                        //   slot:  position in skill array (0..MAX_SKILL_LIST-1)
                        //   skill: skill ID
                        //   level: (m_level << 3) | (m_index & 7)  ← packed
                        //
                        // ANTES: el handler decía "HotbarUpdate" (mal-port) y
                        // solo logueaba. User reportó "no veo skills en la UI"
                        // — los skills nunca llegaban a CharacterAttribute.
                        //
                        // CharacterAttribute.Skill[] layout (per IDA):
                        //   CA[86] = count
                        //   CA[87..86+count] = skill IDs (byte each)
                        //
                        // Hero+913 = SelectedSkill (default 0 = first skill).
```

### Línea 3388 en `Net_ProcessPacket` — antes de `char b[400];`

```cpp
                        // F3/E3 (126B) quest, F3/E4 (854B) skills, F3/E5 (1111B) master tree.
                        // Pendientes — dump-only por ahora (estructuras Protocol.h aún no porteadas).
```

### Línea 3399 en `Net_ProcessPacket` — antes de `const BYTE* p = Msg + ((hdr == 0xC1) ? 4 : 5);`

```cpp
                        // Antes esto solo logueaba como "SkillList" y encima leía Msg+6
                        // (el payload arranca en Msg+4) → el bloque de opciones NUNCA se
                        // aplicaba: el chat window quedaba en el default hardcodeado del
                        // ctor en vez del guardado por personaje.
```

### Línea 3472 en `Net_ProcessPacket` — antes de `char b[420];`

```cpp
                        // 2026-07-25 (#3): dump del payload de CUALQUIER F3 sub
                        // desconocido, para tener la estructura cuando toque
                        // portarlo.  F3/E6 (~326B, trae nombres tipo "Devil
                        // Square") = lista de eventos/GameServer info; F3/E7+ etc.
                        // El server MuEmu los manda en loop; hoy los ignoramos
                        // (inofensivo), pero acá queda el hex para identificarlos.
```

### Línea 3524 en `Net_ProcessPacket` — antes de `case 0x0E:  // LiveClient ACK (server confirma keepalive)`

```cpp
            // BUG-FIX 2026-04-28: el default case ignoraba TODOS los packets
            // in-game. Sin estos handlers, server manda 0x10 (move-confirm),
            // 0x11 (position-set), 0x14 (entity spawn) etc, cliente los descarta
            // → hero estático, NPCs invisibles, movimiento roto.
```

### Línea 3534 en `Net_ProcessPacket` — antes de `case 0x01: {`

```cpp
            // ── CHAT ─────────────────────────────────────────────────────────
            // 2026-07-19: estos tres opcodes NO estaban en el dispatch → todo el
            // chat entrante se descartaba en silencio. Layout autoritativo del
            // server MuEmu (Protocol.h) + confirmado 1:1 con IDA ReceiveChat
            // (0x427630), que lee name en +3 y el mensaje en +13.
            //   C1:00  PMSG_CHAT_SEND         name[10]@+3  message[60]@+13
            //   C1:01  PMSG_CHAT_TARGET_SEND  index[2]@+3  message[60]@+5
            //   C1:02  PMSG_CHAT_WHISPER_SEND name[10]@+3  message[60]@+13
            // 2026-07-27 FIX (el guardia no responde): PMSG_CHAT_TARGET_SEND.
            // El server manda el mensaje de un NPC "hablado" por acá
            // (GCChatTargetSend — NpcTalk::NpcGuard lo usa para el guardia), pero
            // NO había handler in-game → se descartaba en silencio y clickear al
            // guardia sólo hacía el sonido de UI. Layout: index[2]@+3 (BE),
            // message[60]@+5. Se muestra como burbuja de chat sobre la entidad.
```

### Línea 3570 en `Net_ProcessPacket` — antes de `if (Size >= 14 && DAT_07e11dac == 0) {   // m_bBlockWhisper (toggle F3)`

```cpp
                // FIX 2026-07-19: mismo bug que el case 0x00 — el guard exigía el
                // tamaño MÁXIMO (73) de una struct de longitud VARIABLE
                // (`14 + strlen`). Por esto los `/post` dorados nunca aparecían:
                // CommandManager::GCPostMessageGold los manda por este opcode y un
                // post corto llega con Size ~30.
                // PORT FIEL de IDA `ReceiveWhisper` @ 0x4278F0 (identificada por
                // disasm: la función sin nombre entre ReceiveChat y ReceiveNotice):
                //     a0 ac 1d e1 07  mov al, byte_7E11DAC   ; m_bBlockWhisper
                //     84 c0 / 0f 85   test+jnz → descarta el mensaje
                //     8d 4a 03        lea ecx, [edx+3]       ; name  = buf+3
                //     8d 72 0d        lea esi, [edx+0Dh]     ; msg   = buf+13
                //     6a 26 ...       PlayBuffer(0x26, 0, 0) ; sonido de whisper
                //     6a 00 ...       UIChatLogWindow_AddText(name, msg, 0)
                //
                // FIX 2026-07-19: el canal era **3** (invención mía al agregar el
                // handler, nunca validada). IDA usa **0** → negro sobre fondo
                // celeste (0x9632C8FF), el look clásico de whisper. Por eso los
                // `/post` dorados salían con estilo de chat normal.
```

### Línea 3632 en `Net_ProcessPacket` — antes de `if (Size == 3) {`

```cpp
                // 2026-08-25 FIX (el NPC de crear guild no abria nada): el server
                // manda `[C1][03][54]` (`GCGuildMasterQuestionSend`,
                // Protocol.cpp:1795) al hablar con el Guild Master cumpliendo los
                // requisitos, y aca se lo tragaba `Recv_InventoryClose`.
                //
                // Si NO se cumplen, el server ni siquiera manda esto: contesta un
                // chat o un notice (NpcTalk.cpp:197-220 — ya estas en un guild,
                // nivel insuficiente, resets insuficientes). Por eso probandolo
                // con un personaje que ya tenia guild no se abria nada, y eso es
                // correcto.
                //
                // Mismo criterio que el 0x55 de abajo: se distingue por tamaño.
```

### Línea 3693 en `Net_ProcessPacket` — antes de `BYTE* h = nullptr;`

```cpp
                // BUG-FIX 2026-04-29: actualizar entidad sin importar si es hero u otra.
```

### Línea 3748 en `Net_ProcessPacket` — antes de `NetLog("NET:  → op=0x03 MainCheck challenge size=%d, sending ACK (C3)", Size);`

```cpp
                // 2026-05-05 BUG-FIX CRÍTICO (causa raíz del FD_CLOSE post-F3/03):
                // Server log muestra:
                //   [HackPacketCheck][...][...] Packet encryption error
                //   (Index: 3, Value: -1, Encrypt: [0][1])
                //
                // Server HackPacketCheck.txt: opcode 3 (MainCheck) Encrypt=1.
                // Nuestro client mandaba el ACK como C1 plain con MuEmu byte-XOR
                // → server veía encrypt=0 → mismatch [0][1] → CloseClient.
                // Esto explica por qué SIEMPRE kick post-F3/03: el server manda
                // 0x03 MainCheck challenge en el batch post-CharacterInfo, y
                // nuestro ACK plain dispara hack-detection.
                //
                // Net_SendSmallPacket wrap correcto: chain-XOR + serial counter
                // + SimpleModulus + envelope C3.
```

### Línea 3811 en `Net_ProcessPacket` — antes de `int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;`

```cpp
                // 2026-04-30 (v3): re-enabled with C1/C2 framing fix.
                // Server-emu sends ViewportPlayer as C2 (multi-byte length),
                    // pero nuestro handler anterior leía Msg[3] esperando enmarcado C1
                    // — y ése es el byte de OPCODE para C2. Resultado: count=18 era
                // actually 0x12 (the opcode itself!), causing bad-stride skip.
                //
                // Layout per framing:
                //   C1: [C1][len][op=12][count][entries...]   data@Msg+3
                //   C2: [C2][len_hi][len_lo][op=12][count][entries...] data@Msg+4
```

### Línea 3823 en `Net_ProcessPacket` — antes de `if (!DAT_07abf5d0) {`

```cpp
                // 2026-08-25 FIX (reportado: "al entrar desde char-select, si hay
                // NPCs en la zona no cargan; hay que salir y volver a entrar"):
                // aca se DESCARTABA el viewport entero si el heroe todavia no
                // estaba creado. Al entrar al mundo el server manda el viewport
                // junto con el spawn del heroe, asi que ese `break` tiraba a
                // todas las entidades de la zona y solo aparecian al reentrar.
                //
                // IDA `Combat_PacketDispatch` (0x429690) NO tiene ese guard:
                // crea las entidades con `CreateCharacter` sin mirar al heroe.
                // Lo unico que hace falta es el ARRAY de entidades. El filtro de
                // mas abajo ya tolera `DAT_07abf5d8 == 0` (`heroEnt ? ... :
                // g_HeroKey`) y el bloque del heroe tiene su propio `if (hero)`.
```

### Línea 3840 en `Net_ProcessPacket` — antes de `if (count <= 0 || count > 255) {`

```cpp
                // 2026-08-25 FIX (los monstruos no cargaban al entrar a un mapa poblado):
                // el limite era `count > 30`, una invencion del port. Medido: el viewport
                // inicial de Lost Tower llega con `count=41 size=497` y se descartaba
                // ENTERO —"0x13 SKIP - count=41 out of range"—, asi que no se creaba
                // ninguna de las 41 entidades. Se veian los "Miss" de sus ataques y los
                // `0x18`/`0x10` de sus movimientos con "key not found", pero no habia nada
                // dibujado; solo aparecian los pocos que llegaban despues en viewports
                // chicos (count<=30).
                //
                // IDA (`ReceiveCreateMonsterViewport` 0x42A230, `Combat_PacketDispatch`
                // 0x429690) NO tiene limite: itera `if (ReceiveBuffer[4]) do {...} while`
                // por el count crudo, que es un BYTE. El tope real es 255 y la cota util
                // es la validacion por `Size` que ya esta abajo — que recien ahora es
                // confiable, con el fix del tamaño truncado de este mismo PR.
```

### Línea 3877 en `Net_ProcessPacket` — antes de `WORD entityId = ((WORD)(e[0] & 0x7F) << 8) | e[1];   // strip CREATE bit`

```cpp
                    // 2026-05-05: layout decompilado del binario MuEmu
                    // server (CViewport::GCViewportPlayerSend, asm a1590).
                    // Stride 32 bytes:
                    //   e[0..1]=Key BE, e[2]=PosX, e[3]=PosY,
                    //   e[4..b]=Equipment(8B), e[c..d]=PkLevel,
                    //   e[e]=CtlCode, e[f]=padding,
                    //   e[10..11]=ViewSkillState (LE), e[12..1b]=Name(10B),
                    //   e[1c]=TargetX, e[1d]=TargetY, e[1e]=Path|Dir.
                    // Antes leíamos name desde e[0x10] (off-by-2,
                    // 2 bytes basura + 8 chars de nombre real).
```

### Línea 3895 en `Net_ProcessPacket` — antes de `BYTE* heroEnt = (BYTE*)(uintptr_t)DAT_07abf5d8;`

```cpp
                    // Un refresco de viewport por gate de mapa de MuEmu puede devolver
                    // como eco al personaje local. El original mantiene al Hero como único objeto
                    // para HeroKey; pasarlo por CreateCharacter crea
                    // una segunda copia del jugador, renderizada por separado.
                    // 2026-08-24 (issue #14): el filtro comparaba SOLO contra `g_HeroKey`,
                    // un global del port que se fija una vez en el JoinServer. Si alguna vez
                    // quedara desfasado del Key real de la entidad, la entrada del propio
                    // heroe no entraria aca: `CreateCharacter` reusaria su slot por Key y lo
                    // re-inicializaria SIN pasar por la restauracion de abajo (se perderia,
                    // entre otras cosas, el byte de clase +0x1BC). Comparamos contra los dos.
```

### Línea 3915 en `Net_ProcessPacket` — antes de `if (*(short*)(hero + 2) != 390) {`

```cpp
                            // 2026-08-22: vuelta de una transformacion (anillo).
                            // El 0x45 (GCViewportSimpleChangeSend) convierte al
                            // heroe en monstruo llamando CreateMonster con SU
                            // key; al sacarse el anillo el server manda este 0x12
                            // con el player normal.  IDA no filtra por HeroKey:
                            // `Combat_PacketDispatch` L86 llama
                            // `CreateCharacter(key, 390, x, y, 0.0)` para TODAS
                            // las entradas, y eso reusa el slot por key y lo
                            // recrea como player — ese es el camino de vuelta.
                            // Nosotros salteamos la entrada propia para no
                            // clonar al heroe (nuestro scan por key excluye su
                            // slot), asi que replicamos solo la restauracion.
```

### Línea 3965 en `Net_ProcessPacket` — antes de `float rot = ((float)dir - 1.0f) * 45.0f;`

```cpp
                    // 2026-08-24 (issue #14, deuda): aca habia un scan de
                    // slots propio, o sea una SEGUNDA implementacion de
                    // CreateCharacter, y difería del original en dos puntos:
                    //
                    //  1. `if (slot == (BYTE*)DAT_07abf5d8) continue;` — excluia
                    //     el slot del heroe TAMBIEN del match por Key. IDA
                    //     (`Combat_PacketDispatch` L86) llama
                    //     `CreateCharacter(key, 390, x, y, 0.0)` para TODAS las
                    //     entradas, sin excluir a nadie: si llega el propio
                    //     heroe se reusa SU slot. Con la exclusion, cualquier
                    //     entrada que trajera su Key —y que el filtro de
                    //     `g_HeroKey` de mas arriba no atrapara— caia al primer
                    //     slot libre y clonaba al jugador.
                    //  2. No llamaba `DeleteCloth` antes de reusar un slot
                    //     inactivo, asi que la capa (cloth) de la entidad
                    //     anterior quedaba colgada.
                    //
                    // `CreateCharacter` ya hace las dos cosas y es el port fiel
                    // (Monster.cpp:526). Delegar en el elimina la divergencia:
                    // una sola implementacion de "buscar slot por Key, si no
                    // reusar uno inactivo".
                    //
                    // Su centinela de pool lleno (`base + 366400` = slot 400)
                    // cae dentro del buffer: WinMain aloca 0x764D4 = 529 slots
                    // y el offset random inicial se come a lo sumo 127, o sea
                    // quedan 402 utiles.
```

### Línea 4129 en `Net_ProcessPacket` — antes de `int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;`

```cpp
                // 2026-04-30 (v3): el mismo fix de C1/C2 que en 0x12.
```

### Línea 4133 en `Net_ProcessPacket` — antes de `if (!DAT_07abf5d0) {`

```cpp
                // 2026-08-25 FIX (mismo bug que el 0x12, reportado con los
                // monstruos): se descartaba el viewport entero si el heroe aun
                // no estaba creado. Al entrar al mapa el server manda el
                // viewport junto con el spawn del heroe, asi que los monstruos
                // de la zona no se creaban nunca — se veian los "Miss" de sus
                // ataques pero no habia entidad que dibujar, y solo aparecian
                // al salir y volver a entrar.
                //
                // IDA `ReceiveCreateMonsterViewport` (0x42A230) no tiene ese
                // guard, y este handler no usa `DAT_07abf5d8` en ningun lado de
                // su cuerpo: lo unico que necesita es el ARRAY de entidades.
```

### Línea 4154 en `Net_ProcessPacket` — antes de `const int entryStride = 12;`

```cpp
                // 2026-09-03 -- el stride es FIJO 12, no derivado del tamano.
                // IDA `ReceiveCreateMonsterViewport` (0x0042A230): `Data2 =
                // ReceiveBuffer + 7;` y al final del cuerpo del do-while
                // `Data2 += 12;`.  El port lo calculaba como
                // `(Size - entryStart) / count` con un fallback a 10, asi que
                // cualquier desajuste de `Size` desalinea TODAS las entradas
                // desde la segunda: el `type` sale de un byte que no es el suyo
                // y se crean monstruos fantasma (de ahi el "Giant" en un mapa
                // sin spawn, con un id que el server no conoce y por eso
                // inmatable).  Ahora es literal.
```

### Línea 4168 en `Net_ProcessPacket` — antes de `WORD entityId = ((WORD)(e[0] & 0x7F) << 8) | e[1];`

```cpp
                    // 2026-05-05: layout decompilado del binario MuEmu
                    // Linux server (CViewport::GCViewportMonsterSend, asm
                    // a17c0). Confirmado disasm:
                    //   e[0]=KeyH|CREATE, e[1]=KeyL, e[2]=Class,
                    //   e[3]=padding, e[4..5]=ViewSkillState (LE),
                    //   e[6]=PosX, e[7]=PosY, e[8]=TargetX, e[9]=TargetY,
                    //   e[10]=Path|Dir, e[11]=padding.
                    // Antes leíamos x=e[5] y=e[6] (off-by-1) → monsters
                    // spawneaban en pos=(0,N) con N=TargetX en lugar de
                    // PosX/PosY reales.
```

### Línea 4185 en `Net_ProcessPacket` — antes de `extern char* __cdecl CreateMonster(unsigned int Type, int PosX,`

```cpp
                    // 2026-05-04: usar CreateMonster en vez de
                    // CreateCharacterPointer. El primero ADEMÁS
                    // carga el BMD model via OpenMonsterModel/OpenNpc, que es
                    // lo que faltaba — antes los slots se creaban "vacíos"
                    // sin modelo → no rendían en pantalla.
```

### Línea 4200 en `Net_ProcessPacket` — antes de `slot[0x160] = 1;    // visible flag`

```cpp
                        // FIX 2026-07-25: NO sobrescribir +0x84 (kind).  CreateMonster
                        // (CreateMonster) ya lo setea correcto por Type: 2=monster,
                        // 4=NPC (type>200), 8=ground-item.  El override a 2 forzaba a
                        // TODOS los NPCs (blacksmith, mage, etc) a kind=2 → Target_Render
                        // los mostraba como banner de monstruo (RenderCenteredText arriba)
                        // en vez del chat flotante de NPC (CreateChat).
```

### Línea 4215 en `Net_ProcessPacket` — antes de `*(float*)(slot + 0x168) = 1.0f;`

```cpp
                        // 2026-05-06: init +0x168 (screen distance) a 1.0f para
                        // que mob sea targetable INMEDIATAMENTE (antes del primer
                        // frame de render). Sin esto, FUN_004afdc0 (hover detect)
                        // filtra mob por `_DAT_00552580 < ent[+0x168]` (= 0
                        // por default) → mob no es hovered hasta que sea
                        // rendered (1+ frame later).
```

### Línea 4222 en `Net_ProcessPacket` — antes de `if (*(unsigned short*)(slot + 0x2FA) == 0) {`

```cpp
                        // 2026-05-05: init move speed (+0x2FA = +762 word).
                        // CreateMonster solo setea esto para case 11. Resto de
                        // los tipos quedan en 0 → CharacterMoveSpeed retorna 0 →
                        // sin movimiento. O queda en basura → mobs acelerados.
                        // 4 = "slow walk" baseline para mobs/NPCs (vs player ~12).
```

### Línea 4262 en `Net_ProcessPacket` — antes de `if (Size < 7) {`

```cpp
                // 2026-05-06 BUG-FIX MAYÚSCULO: opcode 0x15 server→cliente NO es
                // "ViewportDestroy" (eso era un mal-port de Ghidra). Es
                // GCDamageSend → PMSG_DAMAGE_SEND per
                // Mu-linux-97K/Source/MuServer/GameServer/Protocol.cpp:1595-1626.
                //
                // Layout (basic, 7 bytes; con GAMESERVER_EXTRA es 15):
                //   [C1][07][0x15]
                //   [3] = (target_idx_hi & 0x7F) | (kill_flag << 7)
                //   [4] = target_idx_lo
                //   [5] = (damage_hi & 0x0F) | (damage_type & 0xF0)
                //   [6] = damage_lo
                //
                // El kill_flag (bit 7 de Msg[3]) es CRÍTICO: cuando vale 1, este
                // hit MATÓ al target. El cliente debe entonces:
                //   - Setear dead_flag (+0x34e = 1) para que no sea targetable
                //   - Disparar animación de muerte
                //   - Mostrar "+EXP / -damage" en HUD (placeholder por ahora)
                //
                // ANTES: case 0x15 leía Msg[3] como `count` y trataba el packet
                // como una lista de entity IDs a borrar. Esto significaba que
                // el bit 7 (kill_flag) hacía `count = 128+` → OOB read sobre el
                // packet, y los mobs muertos seguían targetables (user reportó
                // "le pegue hasta animación de muerte pero podía seguir
                // pegando", screenshot 2026-05-06).
                // 2026-05-06: port FIEL desde IDA mu97k-src-IDA/raw/
                // 0042ACC0_ReceiveAttackDamage.c. Parses damage + color flags,
                // le baja HP al héroe si el objetivo es el héroe, y llama a CreatePoint
                // (FUN_004792c0) para spawnear los números de daño flotantes en el espacio del mundo.
```

### Línea 4307 en `Net_ProcessPacket` — antes de `if (Size >= 16) {`

```cpp
                // 2026-08-15 BUG-FIX (el daño se mostraba truncado: un crítico
                // de 12392 salía como 104, y el daño normal saturaba en ~4000).
                //
                // El campo legacy `damage[2]` sólo lleva 12 BITS: el server hace
                //     damage[0] = (SET_NUMBERHB(dmg) & 0x0F) | (type & 0xF0);
                //     damage[1] =  SET_NUMBERLB(dmg);
                // o sea el nibble ALTO de damage[0] es el tipo y sólo quedan 4
                // bits para el byte alto del daño → máximo 0x0FFF = 4095, y por
                // encima de eso se pierden los bits 12+. Comprobado con el caso
                // real: 12392 = 0x3068 → HB=0x30, `& 0x0F` = 0, LB = 0x68 = 104.
                //
                // Con `GAMESERVER_EXTRA=1` (nuestro server: el paquete llega con
                // size=16) el valor REAL viaja sin truncar en `ViewDamageHP`.
                // Layout de PMSG_DAMAGE_SEND (Protocol.h:203), con el padding
                // que mete el DWORD:
                //     +0..2  header      +3,+4  index[2]
                //     +5,+6  damage[2]   +7     PADDING
                //     +8..11 ViewCurHP   +12..15 ViewDamageHP
```

### Línea 4355 en `Net_ProcessPacket` — antes de `if (stunFlag) {`

```cpp
                //
                // El port tenia la cobertura INVERTIDA: gateaba la unica llamada a
                // SetPlayerShock con `!stunFlag`, o sea no hacia nada justo cuando
                // el original aturde incondicionalmente.
```

### Línea 4429 en `Net_ProcessPacket` — antes de `if (damage > 0 && !stunFlag &&`

```cpp
                // ── Hit reaction (anim + grunt) — SetPlayerShock ─────────────
                // 2026-05-08: imported from companion-DLL `IgnoreRandomStuck`
                // patch (Patchs.cpp). The original 0.97k client rolls a 50/50
                // el chequeo aleatorio adentro de ReceiveAttackDamage y llama a
                // SetPlayerShock incondicionalmente cuando la tirada pasa — eso
                // produce el bug del "random stuck", donde el héroe se traba en la
                // anim de shock 130 en medio del combate. El DLL companion hookea el
                // call site para saltearlo cuando la entidad es el jugador (tipo 390).
                // Reproducimos el mismo comportamiento acá: los monstruos reciben
                // su anim de shock + el quejido; el jugador NO.
```

### Línea 4503 en `Net_ProcessPacket` — antes de `if (Size < 7) break;`

```cpp
                // 2026-05-06: port FIEL desde IDA mu97k-src-IDA/raw/
                // 0042B4F0_ReceiveAction.c. Server PMSG_ACTION_SEND format:
                //   struct {
                //     PBMSG_HEAD header;  // [C1][size][0x18]
                //     BYTE index[2];      // [3..4] entity index BIG-endian
                //     BYTE dir;           // [5] direction (1..8) → angle = (dir-1)*45
                //     BYTE action;        // [6] action code (NOT raw anim_state)
                //   };
                //
                // BUG-FIX MAYÚSCULO: el handler viejo leía `action = Msg[5]`
                // (el dir byte) y lo escribía DIRECTAMENTE como anim_state. Eso
                // causaba que el hero después de cada attack del server
                // quedara con anims raros (porque dir=1..8 mapea a glyphs aleat).
                //
                // Action codes per IDA (con sufijo "(walk)" si bit 1 de c+444==2):
                //   18  → emote 92 (sound 81)
                //   100 → SetPlayerAttack (atk1) — dispatched por weapon
                //   101 → SetPlayerAttack (atk2)
                //   102/103 → SetPlayerStop (cancel)
                //   108..125 → emotes: pares walk/idle (anim 93..122)
                //   126..131 → special anims 123..127
                //   else → SetAction(c, raw action)
```

### Línea 4526 en `Net_ProcessPacket` — antes de `WORD entityKey = (WORD)((Msg[3] << 8) | Msg[4]);`

```cpp
                // 2026-05-07 BUG-FIX: IDA ReceiveAction:14 NO maskea bit 7 de
                // Msg[3]. Es el byte alto del Key completo (16 bits). Antes
                // hacíamos `& 0x7F` pensando que era kill_flag (eso es 0x15,
                // NO 0x18). Resultado: cuando el server enviaba un 0x18 con
                // bit 7 del high byte set en el key, fallaba el match con
                // +0x1dc del slot real → caía a `slot=nullptr` y bail-out, OR
                // matcheaba un slot equivocado (otro entity cuyo +0x1dc por
                // casualidad coincidía con el key masked) → ANIMABA EL ENTITY
                // EQUIVOCADO. User reportó: "el hero ataca solo cuando otro
                // mob/player ataca cerca".
```

### Línea 4674 en `Net_ProcessPacket` — antes de `if (Size < 8) break;`

```cpp
                // PacketHandler_0x19 Skill — server tells client about skill effects.
                // Format: [C1][size][0x19][skill_idx][src_id_hi][src_id_lo][tgt_id_hi][tgt_id_lo]
                // 2026-05-07: delega en PacketHandler_0x19 de Skills.cpp, que tiene
                // the full 30+ skill type dispatch (Poison/Ice/Lightning/Combo/etc.).
                // El inline mínimo setea el lock de objetivo + el flag skill_active como respaldo.
```

### Línea 4697 en `Net_ProcessPacket` — antes de `if (Size <= countOff || !DAT_07abf5d0) break;`

```cpp
                // 2026-08-25: idem 0x12/0x13 — el `!DAT_07abf5d8` descartaba el
                // paquete al entrar al mapa. Este handler tampoco usa el heroe.
```

### Línea 4956 en `Net_ProcessPacket` — antes de `entity[405] = 0;`

```cpp
                // 2026-09-04: este bloque faltaba entero, junto con el
                // clearMatchInfo() del heroe.
```

### Línea 5012 en `Net_ProcessPacket` — antes de `NetLog("NET:  → 0x24 ItemMoveSend result=%02X slot=%d size=%d",`

```cpp
                // 2026-05-09: Server response to PMSG_ITEM_MOVE_RECV (client
                // manda 0x24 para pedir un movimiento; el server responde con el mismo
                // opcode confirming or denying).
                //
                // Per server ItemManager.h:100 PMSG_ITEM_MOVE_SEND:
                //   PBMSG_HEAD header;   // C3:24 (3 bytes)
                //   BYTE result;         // 0xFF = denied, else success (= target slot)
                //   BYTE slot;           // target slot
                //   BYTE ItemInfo[4];    // wire-format item bytes
                //
                // After our C3 unwrap, Msg layout: [hdr=C1][size][24][result]
                //   [slot][ItemInfo×4]. So:
                //   Msg[3] = result
                //   Msg[4] = slot
                //   Msg[5..8] = ItemInfo
```

### Línea 5046 en `Net_ProcessPacket` — antes de `BYTE targetSlot = Msg[4];`

```cpp
                        // Server confirmed the move. Msg[4] = target slot,
                        // Msg[5..8] = ItemInfo (4 bytes, ItemByteConvert).
                        //
                        // 2026-07-27 FIX "item se transforma en otro al moverlo":
                        // el port anterior armaba el item mezclando 12 bytes del
                        // ITEM struct agarrado (DAT_07e91350) con 4 bytes wire del
                        // server. El ITEM struct NO está en formato wire — sus
                        // bytes 4-11 (Durability/Option1/x/y/Key…) se
                        // reinterpretaban como opciones/serial del item → el slot
                        // quedaba con type/opciones equivocadas = "otro item".
                        // Ahora construimos el item SOLO desde los 4 bytes wire
                        // del server (autoritativo), igual que el snapshot F3/10
                        // y el buy 0x32. InsertInventoryItem lee hasta Item[4];
                        // dejamos ext=0.
```

### Línea 5168 en `Net_ProcessPacket` — antes de `NetLog("NET:  → 0x30 ReceiveTalk type=%d size=%d", Msg[3], Size);`

```cpp
                // ── ReceiveTalk (IDA 0x4301B0, server→client) ────────────────
                // 2026-07-25 (#2 shops): el server ordena abrir la ventana de un
                // NPC al hablarle. Msg[3] = tipo:
                //   2 = Warehouse (baúl)   3 = Chaos Machine (mezcla)
                //   4/6 = Event window     5 = Server division
                //   default = Shop (comprar/vender)
                // El anti-tamper hash-table que en IDA envuelve el set de
                // ShopOpened se omite per policy — el efecto neto es ShopOpened=1.
                // La rama cliente→server (bEncrypted=0) del IDA es el SEND de la
                // request de "hablar"; nosotros no la usamos (mandamos directo).
```

### Línea 5182 en `Net_ProcessPacket` — antes de `WarehouseOpened = 1;`

```cpp
                        // (Se saco la exclusion mutua de paneles del 2026-07-27: no
                        //  esta en IDA y el click al NPC ya exige ShopOpened == 0 y
                        //  WarehouseOpened == 0.)
```

### Línea 5287 en `Net_ProcessPacket` — antes de `{`

```cpp
                // 2026-07-25 (#2 shops): dump crudo del 0x31 para capturar la
                // estructura de la lista de items de TIENDA (viene C2, offsets
                // distintos del layout C1 que asume este handler). Con esto
                // porteamos la rama shop → pool Inventory[] con los offsets reales.
```

### Línea 5298 en `Net_ProcessPacket` — antes de `{`

```cpp
                // 2026-07-25 (#2 shops) PIEZA C: rama SHOP con los offsets C2
                // reales (IDA ReceiveTradeInventory 0x427560): type=Msg[4],
                // count=Msg[5], records desde Msg[6] stride 5 = [slot(1)][info(4)].
                // El handler viejo de abajo asume layout C1 (Msg[3]/Msg[4], stride
                // 13) — correcto para warehouse C1 pero NO para el shop C2.
```

### Línea 5306 en `Net_ProcessPacket` — antes de `bool isWarehouseList = (Msg[0] == 0xC2) && (listType != 3) && WarehouseOpened;`

```cpp
                    // 2026-07-27 FIX (tienda abre vacía a veces): el gate exigía
                    // `ShopOpened` ya en 1, pero el 0x30 (que lo setea) y el 0x31
                    // llegan casi juntos — si el 0x31 se procesaba antes de que
                    // ShopOpened estuviera seteado, esta rama se saltaba y caía al
                    // handler viejo (layout C1), que limpiaba el pool sin popular
                    // → tienda vacía intermitente (confirmado por el diag SHOPREND:
                    // ShopOpened=1 pos ok pero occ=0). El discriminante correcto es
                    // el FORMATO: header C2 = shop list (stride 5), C1 = warehouse.
                    // 2026-07-27 FIX (el baúl no carga items): el server manda la
                    // lista del BAÚL con el MISMO opcode 0x31 y el MISMO type=0
                    // que la tienda (Warehouse.cpp:276 vs Shop.cpp:251; sólo el
                    // ChaosBox usa type=3). Son indistinguibles por formato, así
                    // que el destino se decide por QUÉ VENTANA está abierta —
                    // como hacía el original. El gate anterior (header C2) mandaba
                    // la lista del baúl al pool de la tienda → baúl vacío.
                    // El 0x30 (que setea Warehouse/ShopOpened) siempre llega ANTES
                    // que el 0x31, así que el flag ya está puesto acá.
```

### Línea 5401 en `Net_ProcessPacket` — antes de `BYTE result = Msg[3];`

```cpp
                // 2026-07-27 FIX: este es el buy-response del shop
                // (PMSG_ITEM_BUY_SEND, ItemManager.cpp CGItemBuyRecv):
                //   [C1][08][32][result][i0][i1][i2][i3]  (Size=8)
                //   result = slot ABSOLUTO del inventario (>=12 = grid
                //            principal en result-12) donde cayó el item,
                //            o 0xFF si la compra falló (sin zen / sin espacio).
                //   i0..i3 = 4-byte ItemInfo (ItemByteConvert): index, level/
                //            opts, durability, hi/exc.
                // El port anterior exigía Size>=16 y usaba el path de item-move
                // (12 bytes) → NUNCA insertaba el item comprado (Size real = 8).
                // Ahora reusa InsertInventoryItem, el mismo path fiel que el
                // snapshot F3/10 (stride 5 = slot + 4 bytes) que sí funciona.
                // IDA ProtocolCore L826: InsertInventoryItem(&Inv, 8, 8, byte[3],
                // body+2, 0).
```

### Línea 5448 en `Net_ProcessPacket` — antes de `NetLog("NET:  → 0x33 TradeAck sub=%d", Msg[3]);`

```cpp
                // 2026-05-08: slot de trade aceptado por el server. Limpia el
                // item flag (server confirmed the swap completed).
                // Per IDA ProtocolCore L834-845.
```

### Línea 5556 en `Net_ProcessPacket` — antes de `NetLog("NET:  → 0x38 SlotClear slot=%d", Msg[3]);`

```cpp
                // 2026-05-08: UI_Main slot-clear notification. Per IDA L1126-1128.
                // Server tells client to clear inventory slot pkt[3].
```

### Línea 5671 en `Net_ProcessPacket` — antes de `NetLog("NET:  → 0x46 TerrainTileUpdate size=%d", Size);`

```cpp
                // 2026-05-07: Terrain tile update (Terrain_TileUpdate in Party.cpp).
                // Sub-type at pkt[3]: 0x00 = rect update, 0x01 = single tile.
```

### Línea 5705 en `Net_ProcessPacket` — antes de `const int kHeaderSize = 16;`

```cpp
                // ReceiveGuildList @ 004348B0. MuEmu Guild.h defines the
                // native long frame as:
                // [C2][sizeHi][sizeLo][52][result][count][TotalScore:DWORD]
                // [score] followed by count * { name[10], number, connected }.
                // La rama 0x65 existente es de otro protocolo y
                // no se puede usar acá porque los offsets de sus campos difieren.
                // 2026-08-15 BUG-FIX (el panel abría con el nombre del guild pero
                // la lista de miembros salía vacía): los offsets estaban corridos
                // 5 bytes por el PADDING de la struct del server. MuEmu Guild.h:
                //     struct PMSG_GUILD_LIST_SEND {
                //         PWMSG_HEAD header;   // C2:52   +0..3
                //         BYTE  result;        //         +4
                //         BYTE  count;         //         +5
                //                              //         +6,+7  PADDING
                //         DWORD TotalScore;    //         +8..11   (alineado a 4)
                //         BYTE  score;         //         +12
                //     };                       // sizeof = 16
                // Los miembros (`PMSG_GUILD_LIST`, 12 bytes: name[10], number,
                // connected) arrancan en +16, no en +11. Verificado contra el
                // wire real: el nombre "mago" caía en Msg[16].
                // Mismo patrón que el F3/06 de los stats (ver CLAUDE.md).
```

### Línea 5739 en `Net_ProcessPacket` — antes de `GuildList_Clear();`

```cpp
                // 2026-08-15: alimentar el WIDGET de lista (dword_55C9FF4), que
                // es de donde `RenderGuildList` saca las filas.  Antes sólo se
                // llenaba `byte_7E919BC` (que el render usa nada más que para el
                // nombre del guild en el título), así que el listado salía vacío.
                // Fiel a IDA ReceiveGuildList @0x4348B0: vtable[10] para limpiar
                // y vtable[28] por cada miembro, con el registro de 13 bytes
                //   +0..9 name · +10 NUL · +11 connected · +12 party (o -1).
```

### Línea 5908 en `Net_ProcessPacket` — antes de `case 0x8E: { // Devil Square admission levels (GameServer extension)`

```cpp
            // ── 0x8E-0x99 — EVENTOS, no guild ─────────────────────────────
            // Hasta 2026-08-26 estos casos llamaban a handlers de guild
            // (Guild_CreateOk, Guild_AddMemberResult, ...) que el port se
            // invento. IDA y MuEmu coinciden en que son eventos: ver la tabla
            // completa en la cabecera de `src/Net/Net_Events.cpp`.
            // El guild de verdad esta en 0x50-0x56, mas arriba en este mismo
            // switch, y no se toca.
```

### Línea 5992 en `Net_ProcessPacket` — antes de `if (Size < 13) { NetLog("NET:  -> 0x9B MatchState size=%d (corto)", Size); break; }`

```cpp
                //
                // 2026-09-04: este case NO EXISTIA en el dispatcher, o sea el
                // panel del evento nunca recibia datos.
```

### Línea 6145 en `Net_ProcessPacket` — antes de `if (Size < 9) break;`

```cpp
                // 2026-05-07: ReceiveMagicPosition @ 0x0042D780 (port FIEL).
                // Server broadcasts a magic-area-attack:
                //   Msg[3..4] = caster entity ID (BE)
                //   Msg[5..6] = ID del skill mágico (BE) (se usa para el efecto visual)
                //   Msg[7]    = direction byte (unused in viz)
                //   Msg[8]    = target count
                //   Msg[9+i*2..10+i*2] = target IDs (BE)
                // Por cada objetivo: anim de shock + popup de daño.
```

### Línea 6274 en `Net_ProcessPacket` — antes de `++g_WorldLoading;`

```cpp
                        // 2026-09-02 (monstruos que "cargan mal" al entrar a un mapa): OpenWorld
                        // tarda ~2 s cargando BMDs y, para que el server no cierre por backpressure,
                        // AccessModel pumpea la cola de mensajes cada 8 modelos.  Ese pump entrega
                        // WM_USER -> Net_Recv -> **Net_ProcessPacket**, o sea los handlers corren
                        // RE-ENTRANTES en mitad de la carga: el `0x13 ViewportMonster` creaba
                        // monstruos cuyo modelo todavia no estaba abierto (visto en debug.log: el
                        // spawn del slot 0 cae entre Object01.bmd y Object42.bmd).  De ahi que
                        // salieran mal y que alejarse y volver -- que los re-crea con el modelo ya
                        // cargado -- los arreglara.
                        //
                        // `g_WorldLoading` deja que el pump siga DRENANDO el socket (que es lo que
                        // evita el backpressure) pero suspende el dispatch: los paquetes quedan en la
                        // cola y se procesan al terminar la carga.
```

### Línea 6304 en `Net_ProcessPacket` — antes de `{`

```cpp
                    // ── ACK de fin de carga: C1 04 F3 12 ──────────────────
                    // 2026-08-15 BUG-FIX (el `/move` sólo funcionaba una vez y
                    // el mapa nuevo quedaba sin NPCs ni mobs).
                    //
                    // IDA `ReceiveTeleport` @0x428210, dentro del branch de gate
                    // y DESPUÉS de OpenWorld, arma y envía un paquete
                    // (`v118[2]=0xC1 v118[3]=1 v118[4]=0xF3` + chain-XOR) y
                    // recién entonces setea `LoadingWorld = 30`. Nuestro port
                    // hacía el ClearItems/ClearCharacters/OpenWorld pero nunca
                    // enviaba el ACK.
                    //
                    // Del lado del server (MuEmu) el ciclo es:
                    //   gObjMoveGate OK        → RegenOk = 1  (User.cpp:1964)
                    //   cliente manda F3/12    → RegenOk = 2  (Protocol.cpp:1439
                    //                            CGCharacterMoveViewportEnableRecv)
                    //   tick de ObjectManager  → RegenOk = 3, State = OBJECT_CREATE,
                    //                            aplica RegenMapNumber/X/Y y recién
                    //                            ahí manda el viewport del mapa nuevo
                    //   luego                  → RegenOk = 0
                    //
                    // Sin el ACK, `RegenOk` se queda en 1 y produce los DOS
                    // síntomas a la vez:
                    //   · `gObjMoveGate` (User.cpp:1882) hace
                    //     `if (lpObj->RegenOk != 0 ...) goto ERROR_JUMP;` — todo
                    //     `/move` posterior se rechaza, y el ERROR_JUMP reenvía
                    //     la posición ACTUAL, que el cliente interpreta como un
                    //     teleport al mismo lugar (de ahí "siempre va al primer
                    //     destino").
                    //   · nunca se llega a `State = OBJECT_CREATE`, así que el
                    //     server no manda las entidades del mapa: sin NPCs ni
                    //     mobs.
```

### Línea 6392 en `Net_ProcessPacket` — antes de `if (Size < 4) break;`

```cpp
                // 2026-05-07: ReceiveDropItem @ 0x0042F690 (port FIEL).
                // Server response after hero drops/moves an item.
                //   Msg[3] == 0  → drop FAILED → reset inventory drag UI
                //   Msg[3] != 0  → drop OK
                //     If Msg[4] >= 12 → equipment slot update (UI_Main path)
                //     Else            → inventory slot drop (CharacterMachine update)
                //   En los dos casos de éxito: limpia DAT_07e91388 (arrastre activo)
                //   and reset SendDropItem = -1
```

### Línea 6403 en `Net_ProcessPacket` — antes de `if (result == 0) {`

```cpp
                // 2026-07-27 FIX: el item se sacó del slot de origen al hacer
                // pickup (UI_Main limpia el footprint). Por eso:
                //  - result==0 (drop rechazado por el server): hay que
                //    RESTAURAR el item a su slot de origen o desaparece de la
                //    vista (el server no lo removió). El port anterior sólo
                //    limpiaba el cursor → item perdido visualmente.
                //  - result!=0 (drop OK): el server removió el item; sólo hay
                //    que soltar el cursor. El port anterior escribía a globales
                //    equivocados (DAT_07ea9328 / CharacterMachine+552) con lógica
                //    slot>=12 invertida → corrupción; el slot ya estaba vacío
                //    desde el pickup así que esos writes eran innecesarios.
```

### Línea 6427 en `Net_ProcessPacket` — antes de `if (Size < 4) break;`

```cpp
                // 2026-05-07: ReceiveGetItem @ 0x0042F360 (port FIEL).
                // Respuesta del server cuando el héroe levanta un item del piso vía el envío 0x22.
                //   Msg[3] == 0xFF → pickup failed (no inventory space, etc.)
                //   Msg[3] == 0xFE → levanta zen; el monto es el DWORD BE en Msg[4..7]
                //                    stored at CharacterMachine + 0x548
                //   si no          → item en el slot Msg[3], datos en Msg[4..]
```

### Línea 6457 en `Net_ProcessPacket` — antes de `if (Size >= 8 && slot < 76) {`

```cpp
                    // 2026-07-27 FIX (item levantado no aparecía en inventario):
                    // PMSG_ITEM_GET_SEND es [C3][08][22][result][i0..i3] = Size 8
                    // (ItemInfo son 4 bytes, MAX_ITEM_INFO). El gate `Size >= 16`
                    // (formato de 12 bytes) NUNCA se cumplía → el item se
                    // levantaba en el server pero jamás se insertaba en el grid.
                    // Mismo bug que tenía el buy 0x32. InsertInventoryItem lee
                    // hasta Item[4]; dejamos ext=0.
```

### Línea 6472 en `Net_ProcessPacket` — antes de `if (slot != 0xFF && Item != nullptr) {`

```cpp
                // 2026-08-21: la rama del zen tenía PlayBuffer(49) hardcodeado
                // ("jewel pickup sound"), que es invención del port — al levantar
                // zen sonaba la joya en vez del pickup normal.
```

### Línea 6490 en `Net_ProcessPacket` — antes de `if (Size < 6) break;`

```cpp
                // 2026-05-07: ReceiveLife @ 0x00431780 (port FIEL).
                // Server pushes HP / MaxHP updates and item-durability decrements
                // por este opcode. El sub-byte en Msg[3] elige:
                //   0xFD     → EnableUse = 0 (item slot lock)
                //   0xFE     → MaxLife (CharacterAttribute+32) = WORD BE
                //   0xFF     → Life    (CharacterAttribute+28) = WORD BE
                //   else     → Inventory slot N decrement: ItemAttribute[N].Durability--
```

### Línea 6526 en `Net_ProcessPacket` — antes de `if (Size < 8) break;`

```cpp
                // 2026-05-07: ReceiveMana @ 0x00431A90 (port FIEL).
                // Server pushes Mana / BP / MaxMana / MaxBP updates.
                //   0xFE → MaxMana(offset 34) + MaxBP(offset 38), cada uno WORD BE
                //   0xFF → Mana(offset 30)    + BP(offset 36),    cada uno WORD BE
```

### Línea 6558 en `Net_ProcessPacket` — antes de `if (Size < 5) break;`

```cpp
                // 2026-06-02: borrado de slot del lado del server. Lo usa mucho
                // stackable moves (source slot emptied after merge).
```

### Línea 6597 en `Net_ProcessPacket` — antes de `if (Size < 6) break;`

```cpp
                // 2026-06-02: actualización de durabilidad/cantidad del lado del server. La usa
                // stackable potions/jewels after partial merge.
```

### Línea 6629 en `Net_ProcessPacket` — antes de `if (Size < 6) break;`

```cpp
                // 2026-05-07: ReceiveDurability @ 0x00431EA0 (port FIEL).
                // Per-equipment-slot durability update.
                //   Msg[3] = inventory slot index (0..11)
                //   Msg[4] = new durability value
                //   Msg[5] = if non-zero, EnableUse = 0 (item just consumed/broke)
                // El original escribe en CharacterMachine + 68*slot + 562, que es
                // CharacterMachine.EquipmentSlots[slot].Durability.
```

### Línea 6830 en `Net_ProcessPacket` — antes de `if (Size >= 4 && Msg[3] == 0)`

```cpp
                // 2026-09-12: el opcode no tenia handler.
```

### Línea 6841 en `Net_ProcessPacket` — antes de `if (Size < 5) {`

```cpp
                // 2026-05-06 BUG-FIX MAYÚSCULO (port FIEL desde IDA
                // mu97k-src-IDA/raw/00427A00_ReceiveNotice.c):
                //
                // PMSG_NOTICE_SEND server layout (server source
                // Mu-linux-97K/Source/MuServer/GameServer/Notice.cpp):
                //   struct { PBMSG_HEAD header; BYTE type; char message[256]; }
                //   = [C1][size][0x0D][type][message null-terminated]
                //
                // type 0 → CreateNotice(msg, 0) — CENTERED blue/cyan banner con
                //          blink (UI_AddNotice). Eventos del servidor (Blood
                //          Castle, Happy Hour, server close, etc).
                // type 1 → UIChatLogWindow_AddText — BLUE chat log local
                //          (mensajes personales: login welcome, errors, etc).
                // type 2 → guild notice (sprintf "Guild: %s" + CreateNotice
                //          gold). Deferred — guild stack no portado.
                //
                // ANTES: parser leía message desde Msg+7 (offset incorrecto, mal
                // port basado en formato 5.2 con extra fields). Y ruteaba TODO
                // a chat log → eventos aparecían en azul abajo-izquierda en vez
                // de centrados en pantalla. User reportó este bug 2026-05-06.
```

### Línea 6910 en `Net_ProcessPacket` — antes de `if (Size >= 4) {`

```cpp
                // 2026-05-07: port FIEL desde IDA ProtocolCore:554
                //   case 0xF:
                //     Weather = ReceiveBuffer[3];
                //     if (Weather >> 4) {
                //       if (Weather >> 4 == 1) RainTarget = 6 * (Weather & 0xF);
                //     } else RainTarget = 0;
                //
                // Server controls weather state per map. High nibble = weather
                // type (0=clear, 1=rain). Low nibble = intensity (0..15).
```

### Línea 6937 en `Net_ProcessPacket` — antes de `NetLog("NET:  → 0x00 Chat size=%d", Size);`

```cpp
                // PMSG_CHAT_SEND server→client — [C1][size][0x00][name 10B][msg 60B]
                // 2026-07-19: delega al port FIEL `ReceiveChat` (IDA 0x427630).
                // El handler anterior aproximaba mal: pasaba nullptr como nombre y
                // pre-formateaba "name: msg" (IDA los pasa SEPARADOS — el renderLine
                // del ChatListBox compone "name: text"), usaba canal 0 en vez de 3
                // (color equivocado), y no hacía el dispatch de prefijos
                // ('~'=party/4, '@'=guild/5, '#'=solo burbuja) ni la burbuja
                // sobre el personaje (AssignChat).
```

### Línea 6946 en `Net_ProcessPacket` — antes de `extern void __cdecl ReceiveChat(BYTE* ReceiveBuffer);`

```cpp
                // GUARDA 2026-07-19: el server manda `C1 04 00 xx` (4 bytes) como
                // ping/handshake. ReceiveChat lee name@+3 y mensaje hasta +72, así que
                // con 4 bytes sobre-lee. El IDA solo ACKea esos en estado login
                // (SceneFlag==2) y cae al parseo de chat en el resto → mismo
                // sobre-lectura. Procesamos como chat solo si el paquete tiene el
                // tamaño de PMSG_CHAT_SEND (3 hdr + 10 name + 60 msg = 73).
```

### Línea 6953 en `Net_ProcessPacket` — antes de `if (Size >= 14 || SceneFlag == 2) {`

```cpp
                // FIX 2026-07-19: el guard era `Size >= 73` (tamaño MÁXIMO de la
                // struct). PMSG_CHAT_SEND es de longitud VARIABLE — el server hace
                //   header.set(0x00, sizeof(pMsg) - (sizeof(pMsg.message) - (size+1)))
                //   = 14 + strlen(mensaje)
                // así que solo un mensaje de 59 chars llegaba a 73. Todo mensaje más
                // corto se descartaba en silencio; en particular los `/post` azul (`~`)
                // y verde (`@`) de CommandManager::GCPostMessageBlue/Green.
                // Mínimo real = 3 (hdr) + 10 (name) + 1 (al menos un char) = 14.
                // El server null-termina el mensaje, así que la lectura de 60 bytes
                // que hace ReceiveChat (fiel a IDA) se corta sola en el NUL.
```

### Línea 6981 en `Net_ProcessPacket` — antes de `BYTE* itemPool = (BYTE*)&DAT_07e12840[0];`

```cpp
                // 2026-07-27: escribir sobre el ITEM-BASE (DAT_07e127f8), no
                // sobre DAT_07e12840 (= item-base+72). Los offsets de abajo son
                // ip-relativos de CreateItem (ip+4 type, ip+72 active, ip+88 pos);
                // con la base correcta el active queda en ip+72 = DAT_07e12840+0,
                // que es donde Entity_Render lo lee.
```

### Línea 6989 en `Net_ProcessPacket` — antes de `for (int i = 0; i < count && cursor + 9 <= Size; ++i) {`

```cpp
                // 2026-07-27: este server (MuEmu) manda PMSG_VIEWPORT_ITEM =
                // index[2]+x+y+ItemInfo[MAX_ITEM_INFO+1] = 2+1+1+5 = 9 bytes por
                // item SIEMPRE (Viewport.h). El port usaba stride 8 (0.97k) salvo
                // type 463 → desalineaba todos los items después del 1ro. Bound y
                // stride ahora son 9.
```

### Línea 7079 en `Net_ProcessPacket` — antes de `*(int*)(ip + 352) = (int)0xC1F00000;  // -30.0f`

```cpp
                    // 2026-08-21: el port tenía -12.5 / 25.0 (mal decodificados
                    // desde los literales -1041235968 / 1106247680).
```

### Línea 7135 en `Net_ProcessPacket` — antes de `int entryStart = 4 + hdrOff;`

```cpp
                // 2026-05-05: limpiar slots en DAT_07e12840 pool. Per-entry
                // 2 bytes (key WORD, big-endian).
```

### Línea 7150 en `Net_ProcessPacket` — antes de `case 0xA0: {   // ReceiveQuestHistory @ 0x00437450`

```cpp
            // ── 0xA0..0xA3 — sistema de quests ──────────────────────────────
            // 2026-08-21: los cuatro opcodes no estaban en el dispatcher (sólo
            // el comentario del mapa de opcodes al principio del archivo), asi
            // que el estado de quest nunca llegaba aunque los cuerpos ya
            // estuvieran portados.  ProtocolCore (0x4389A0 L1376-1387) los
            // manda a ReceiveQuestHistory / State / Result / Prize.
```

### Línea 7289 en `Net_ProcessPacket` — antes de `case 0xDD: {`

```cpp
            // ── Character config opcodes (DLL Protocol.cpp:308-327) ─────────
            // 2026-05-04: las structs PMSG_CHARACTER_*_RECV usan PBMSG_HEAD (3 bytes)
            // + member alineado al tipo. WORD se alinea a 2 → +1 PAD entre header
            // y data. DWORD se alinea a 4 → +1 PAD igual. Por eso los offsets son
            // 4 para WORD/DWORD (no 3) y la size on-wire es 6/4/8 (no 5/4/7).
            // Confirmado en debug.log: hdr=C1 op=DD size=6, op=DE size=4, op=DF size=8.
```

## `src/Net/PacketFrame.h`

### Línea 1 — antes de `#pragma once`

```cpp
// PacketFrame.h
// Tabla de frames por opcode — replica lo que el GameServer valida en
// CHackPacketCheck::CheckPacketHack (columna `Encrypt` de
// `Data/Hack/HackPacketCheck.txt`).
//
// El problema que resuelve
// -----------------------
// El server exige por opcode un frame concreto:
//
//     Encrypt = 0  ->  el paquete DEBE venir como C1/C2 (plano + chain-XOR)
//     Encrypt = 1  ->  DEBE venir como C3/C4 (serial + chain-XOR + CSimpleModulus)
//     Encrypt = *  ->  cualquiera
//
// Si no coincide: "Packet encryption error" -> CloseClient, o sea el cliente
// se desconecta ~1 s despues de la accion, sin ningun mensaje. Y si el opcode
// no figura en el archivo: "Packet unknown error" -> CloseClient tambien.
//
// Hasta 2026-08-26 el cliente elegia el frame A MANO en cada call site (74
// repartidos entre 5 helpers de envio), asi que cada opcode nuevo era una
// chance de repetir el bug. Ya paso varias veces: reparar (0x34),
// trade-unconfirm (0x3C), guild (0x50/0x51/0x52/0x57) y los botones [+] de
// stats (F3/06) se mandaban con el frame equivocado y desconectaban.
//
// Ahora los helpers consultan esta tabla y corrigen el frame solos.
```

## `src/Net/Pipe_Legacy.cpp`

### Línea 1 — antes de `// stubs_helpers.cpp`

```cpp
// Pipe_Legacy.cpp
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

## `src/Net/SecondPassword.cpp`

### Línea 144 — antes de `extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);`

```cpp
// 2026-08-25: el rango de guild pide C1 plano (Encrypt=0).
```

### Línea 158 — antes de `// IDA: SecondPassword_Handler (0x004E93A0)`

```cpp
// 2026-05-07: B3 refactor — SecondPassword screens (SecondPassword_Screen1 .. FUN_004ec330)
// moved from stubs.cpp lines 6961-8495 (1535 lines). Full implementation below.
```

### Línea 161 — antes de `extern "C" int __cdecl sub_4E9300_(void);   // hit-test del teclado (0x004E9300)`

```cpp
// 2026-09-16: era `return 0`, o sea el teclado no respondia a nada; y el render
// leia variables propias de HUD_Pass3 en vez de estos globales, asi que tampoco
// se dibujaba.  Por eso el boton del candado no hacia nada.
//
```

### Línea 275 — antes de `static bool GetGuildCreatorOrigin(int& originX, int& originY)`

```cpp
// Inventory_DropDispatch @ 0x004DF410 — Inventory drop dispatcher.
// 2026-05-08: port FIEL completo movido a `Item/Item_ClickHandler.cpp`
// (~150 líneas). Antes era stub vacío bloqueando toda la cadena drag-drop.
// Misnamed previously as "SecondPassword_NetTick" — IDA confirma que es
// el dispatcher de drop sobre las 4 inventories abiertas + sell-to-shop +
// drop-on-ground.
// Sub-handlers de SecondPassword (todos usan frame SEH de Windows — sólo stubs)
// Son las funciones de render/tick por frame del subsistema del diálogo de 2da contraseña.
// Cada una tiene entre 80 y 854 líneas decompiladas. Las implementaciones completas están en src/Net/SecondPassword_UI.cpp.
//
```

### Línea 287 en `GetGuildCreatorOrigin` — antes de `originX = g_GuildCreatorScratchX;`

```cpp
    // 2026-07-27: el scratch ya no vive en Inventory[32] (= slot 0 del pool de
    // la tienda, lo pisaba); ahora tiene globals propios.
```

### Línea 299 — antes de `void __cdecl SecondPassword_Screen1(void) {`

```cpp
//   - SEH + llamadas a UI_SetScene(0x19/0x1c). Implementado en SecondPassword_UI.cpp.
```

### Línea 301 en `SecondPassword_Screen1` — antes de `int originX = 0, originY = 0;`

```cpp
    // 2026-05-04: BUG-FIX del sonido de click fantasma del lado izquierdo. El sub_4E4760 de IDA es
    // el hit-test del diálogo GuildCreator (gatea con GuildCreatorOpened, usa
    // Inventory[32].Level/Part as panel origin). Our port had wrong gate
    // (DAT_07eaa124) Y con la base equivocada (DAT_07ea5b1c/20 = 0 → los tests disparan en
    // left-side x=0..190 → phantom click sfx).
    // Hasta que cableemos el storage de Inventory[32] y el camino de render correcto, gateamos
    // además con `DAT_07ea5b1c != 0` para que los hit-tests fantasma del lado izquierdo
    // no disparen nunca. El código de la fase de render en HUD_Pass6 maneja los clicks reales
    // for Char/Party/Guild panels.
```

### Línea 367 en `SecondPassword_Screen1` — antes de `if (DAT_07eaa144) {`

```cpp
            // 2026-08-25 PORT (no se podia crear un guild): este boton — el izquierdo,
            // rect [+20,+90) x [+350,+371) — es el de CREAR, y mandaba el opcode
            // equivocado. IDA `sub_4E4760` L210 manda aca el **0x55**
            // (PMSG_GUILD_CREATE_RECV); el 0x54 es el del boton derecho, que ya esta
            // bien abajo.
            //
            //   struct PMSG_GUILD_CREATE_RECV {   // Guild.h:218
            //       PBMSG_HEAD header;   // C1 : 43 : 0x55
            //       char GuildName[8];   // +3
            //       BYTE Mark[32];       // +11
            //   };
            //
            // `g_iKeyPadEnable` (= DAT_07eaa144, 0x7EAA144) distingue DOS MODOS del
            // mismo panel, no autoriza nada — `ProtocolCore` L1253-1258:
            //     0x54 recibido -> GuildCreatorOpened=1, g_iKeyPadEnable=0
            //                      (dialogo previo "¿crear guild?", OK/Cancel)
            //     0x55 recibido -> GuildCreatorOpened=1, g_iKeyPadEnable=1
            //                      (UI de creacion: nombre + marca)
            // y cada boton cambia de opcode segun el modo (IDA sub_4E4760):
            //     izquierdo: modo 0 -> 0x54 result=1   |  modo 1 -> 0x55 CREAR
            //     derecho:   modo 0 -> 0x54            |  modo 1 -> 0x57 cancelar
            //
            // El `== 0` de este gate era correcto: es la rama del dialogo previo,
            // que ya mandaba bien su 0x54. Lo que faltaba era la OTRA rama.
            //
            // Nota aparte: el flag estaba partido en dos — `HUD_Pass6.cpp` tenia un
            // `static int g_iKeyPadEnable` homonimo que el handler del 0x55 seteaba
            // mientras este hit-test leia el global. Unificados.
```

### Línea 477 en `SecondPassword_Screen1` — antes de `DAT_07db8710[0][0] = 0;`

```cpp
            // 2026-05-04: DAT_07db8710 ahora es char[10][256] — escribimos el primer byte
            // del slot 0 explícitamente. (Los globals 8714/8718 son símbolos separados
            // preexistentes — los limpiamos como antes para terminar en NUL lo que haya quedado
            // username data.)
```

### Línea 498 en `SecondPassword_Screen1` — antes de `if (DAT_055c9ff4 && *(int*)DAT_055c9ff4) {`

```cpp
        // 2026-04-30 BUG-FIX: vtable[+0x14] = slot 5 = ChatLB_tick (__fastcall
        // toma esto en ECX + argumento). El dispatch cdecl estilo Ghidra anterior
        // con el argumento `(0)` dejaba ECX = basura → ChatLB_tick leía memoria random
        // como `self[3]` (contador de list1), entraba al loop de desencolado, y
        // ChatListBox_DequeueFront deferenciaba un puntero-atrás de nodo que eran bytes de código
        // (0x83EC8B5D = pop ebp; mov ebp, esp; ...).
```

### Línea 584 en `SecondPassword_Screen3` — antes de `if (DAT_07eaa116 == '\0' || DAT_07ea982c == 0) return;`

```cpp
    // 2026-05-04: BUG-FIX phantom click. IDA sub_4E5DE0 = Character panel
    // hit-test, usa CharacterInfoStartX/Y como base. Nuestro port usa
    // uninitialized DAT_07ea982c/30 (=0) → fake hit-tests at left side fire
    // PlayBuffer click sfx whenever Character is open. Skip until
    // CharacterInfoStartX también se cablea acá; el RenderCharacterInfoWindow de HUD_Pass6
    // already handles [+] stat-add and other panel clicks.
```

### Línea 612 en `SecondPassword_Screen3` — antes de `if (*(short *)((BYTE*)DAT_07cf1ff4 + 0x54) != 0) {`

```cpp
    // ── Botones [+] de stats ─────────────────────────────────────────────────
    // 2026-08-08 FIX (subir un punto DESCONECTABA): esta rama estaba mal portada
    // en tres cosas y sólo no se notaba porque el early-return por
    // `DAT_07ea982c == 0` la mantenía muerta (CharacterInfoStartX nunca se
    // escribía). Al arreglar el origen del panel, la rama despertó y mandó un
    // paquete basura → el server ve `size < 3` ("Protocol size error") y cierra
    // el socket. En el log: `AUTO-ENCRYPT C1 len=5 plain=[C1 01 00 34 03]`
    // seguido de FD_CLOSE 62 ms después.
    // Lo que estaba mal vs IDA sub_4E5DE0 L85-244:
    //   1. Paso de fila 15 px y 16 filas → son **60 px y 4 filas** (`v8 += 60;
    //      while (v8 < 240)`) = Fuerza / Agilidad / Vitalidad / Energía.
    //   2. El paquete se armaba a mano como `[C1][01][00][xor][row]`. El
    //      original arma `[C1][len][F3][06][statIdx]` (buf[2]=0xC1, buf[4]=0xF3,
    //      el subcode 6 y el índice al final) = PMSG_LEVEL_UP_POINT_RECV
    //      (Protocol.h:128) — y sale por el camino C3 con serial.
    //   3. Se mandaba con `send()` crudo, sin serial ni CSM.
    // Gate `*(short*)(CharacterAttribute + 0x54) != 0` = LevelUpPoint, fiel.
```

### Línea 764 en `SecondPassword_Screen4` — antes de `DAT_07eaa164 = 0;`

```cpp
    // 2026-09-17: faltaba el +200, asi que los casilleros nunca se procesaban.
```

### Línea 772 en `SecondPassword_Screen4` — antes de `FUN_004d23b0((char*)(uintptr_t)(DAT_07ea5288 + 0xf),`

```cpp
        // 2026-05-08: bug-fix — el inv_base era `&DAT_07ea8410` (DWORD de 4
        // bytes en globals.cpp) heredado del IDA. En el binario original,
        // DAT_07ea8410 ES la base del pool de inventario; en nuestro build
        // OffsetInventoryItems es ese pool. Sin este fix el click handler
        // walkeaba 4 bytes de DAT_07ea8410 + globals adyacentes random como
        // si fueran items → "inventario lleno de fantasmas" + sin tooltip.
```

### Línea 818 en `SecondPassword_Screen4` — antes de `FUN_004d23b0((char*)(uintptr_t)(DAT_07ea5290 + 0xf),`

```cpp
        // 2026-05-08: trade — DAT_07ea5298 / DAT_07ea7b88 son DWORDs (4 bytes)
        // en globals.cpp pero en el binario original son las bases de los
        // pools de trade. Tal como 004D23B0: Inventory es la oferta remota
        // de sólo lectura (arriba) y OffsetTradeItems la oferta local
        // editable (abajo).
```

### Línea 830 en `SecondPassword_Screen4` — antes de `DAT_07eaa0c8 = 260;`

```cpp
    // FIX 2026-07-25: el render de shop/warehouse/chaos (RenderShopInterface
    // HUD_Pass6:1358) usa DAT_07eaa0c8=260 / DAT_07eaa0cc=0 (símbolo separado
    // del global DAT_07eaa0c8 en el port). El hover de abajo usa DAT_07eaa0c8,
    // que valía 0 → el tooltip caía en top-left (sx=35) en vez de sobre el item.
    // Sincronizamos el global con los valores del render para que coincidan.
```

### Línea 839 en `SecondPassword_Screen4` — antes de `FUN_004d23b0((char*)(uintptr_t)(DAT_07eaa0c8 + 0xf),`

```cpp
        // FIX 2026-07-25: era copy-paste del branch de Warehouse (usaba
        // OffsetWarehouseItems → el hover leía un slot basura y el tooltip
        // mostraba un item que no está en la tienda, ej "Kris"). El pool de
        // TIENDA es el overlay &Inventory[32].WalkSpeed (offset +24), el mismo
        // que usa el render (HUD_Pass3:382 sub_4E38B0). Grid 8×15.
```

### Línea 860 — antes de `void __cdecl SecondPassword_Screen5(void) {`

```cpp
// SecondPassword_Screen5 @ 0x004E6C40 — SecondPassword_Screen5 (783 lines)
//   - Main second-password entry UI: draws 10-button numeric keypad, handles click
//     (appends digit to DAT_07ea9814 buffer), Enter → sends packet, ESC → cancel.
//   - SEH. Implemented in SecondPassword_UI.cpp.
```

### Línea 865 en `SecondPassword_Screen5` — antes de `if (!EventWindowOpened) return;`

```cpp
    // Click del selector de nivel del evento.  El binario NO tiene aca ningun
    // teclado de PIN: son 4 filas (Devil Square) o 6 (Blood Castle) alineadas
    // con las que dibuja `RenderEventWindow` (0x4F3C50).
    //
    // 2026-09-07: reescrita contra IDA.  Lo que estaba antes eran rects
    // aproximados con globals que no correspondian, y los dos paquetes de
    // entrada armados con la CLAVE XOR anti-tamper que Hex-Rays emite inline
    // (`v59 = -25; v60 = 109; ...` = E7 6D 3A 89 ...) tomada por bytes del
    // paquete -> el slot terminaba valiendo siempre key[4] = 0xBC.  Ademas el
    // chequeo de nivel leia `(&DAT_00559f60)[i*2]`, un int suelto partido en dos
    // respecto de `m_iDevilSquareLimitLevel` (ver globals.h), asi que rechazaba
    // por nivel aun con la entrada correcta.
```

### Línea 1321 en `FUN_004ec330` — antes de `int iX3 = (int)DAT_07eaa0c8 + 0x73;`

```cpp
            // 2026-09-12: estaba portado como un "paquete de PIN" con header de
            // largo 1 que el server no podia interpretar.
```

### Línea 1356 — antes de `// SecondPassword_GridSlotAvail @ 0x004E3DB0 — SecondPassword_GridSlotAvail`

```cpp
// ShowCheckBox (0x0051E240) vive en src/Item/Item_ClickHandler.cpp.
//
// 2026-09-26: aca habia una SEGUNDA implementacion bajo el nombre ShowCheckBox.
// Las dos portan la misma funcion, pero difieren en la rama del mensaje 153
// (0x99): IDA arma el rotulo con GlobalText[166..169] segun el tipo de huevo de
// mascota (item 431), y esta copia usaba unos strings sueltos DAT_005618b8..c4.
// La de Item_ClickHandler.cpp coincide termino por termino con el decompile
// -- incluidos los descriptores de boton {1,21,90,70,21} y {3,120,90,70,21} --
// asi que se queda esa y sus 3 call sites pasan a llamarla.
```

### Línea 1366 — antes de `uint __cdecl SecondPassword_GridSlotAvail(int a1, int a2, int a3, int a4, int a5)`

```cpp
// SecondPassword_GridSlotAvail @ 0x004E3DB0 — SecondPassword_GridSlotAvail
// Escanea una grilla 2D (param_3×param_2 filas/columnas) en el array de inventario en param_1,
// con stride 0x44 por celda; devuelve 1 si alguna celda de la ventana está libre (slot==-1), si no 0.
// SecondPassword_GridSlotAvail (IDA-activated, was Ghidra stub)
```

### Línea 1463 en `SecondPassword_CancelReturn` — antes de `short *psVar2 = (short *)&DAT_07ea9848;`

```cpp
    // 2026-09-08: el bound `< 0x7eaa0c8` es una direccion absoluta del binario
    // fuente.  Es el mismo pool de 32 slots de 0x44 que resetea Net_PacketSession
    // (DAT_07ea9880), abordado 0x38 antes: (0x7EAA0C8 - 0x7EA9848) / 0x44 = 32,
    // o sea 4 vueltas del bucle externo por 8 del interno.
```

### Línea 1540 — antes de `extern "C" void __cdecl SyncPickedItemVisualState(void);`

```cpp
// FUN_004d1fc0 @ 0x004D1FC0 — SecondPassword_WidgetGrid_Init
// Inicializa la grilla del widget de segunda contraseña insertando / actualizando entradas en la
// hash-table global (MAIN_HASH_CLASS) con clave DAT_07cf1ffc, y después llama a FUN_004cdc70 para
// place 12 grid-slot widgets at fixed screen positions:
//   slot 0-1 : (700,184)  (380,184) size 40×40 / 60×40
//   slot 2-4 : (300,360)  (300,400) (180,360)
//   slot 5-6 : (390,400)  (198,400) size 40×40
//   slot 7   : checkbox (190,360)   wait (190,400)
//   slot 8-10: (350,360)  (350,400) (380,400) size 20×20
//   slot 11  : (180,400)
// Usa HashTable_GetIndex / HashTable_Insert / Packet_DecryptBuffer con el ref-count ofuscado por XOR
// sobre el blob de widget de 0x584 bytes. La clave XOR sale de DAT_00559050 (tabla de 16 bytes).
// STUB: la lógica real son 12 llamadas a FUN_004cdc70 (función de render de inventario de 3554 líneas, sin declarar)
// setting up second-password widget grid at fixed screen coordinates.
// No se puede implementar hasta que FUN_004cdc70 esté declarada en functions.h.
// Widget positions (hex float → decimal):
//   slot 8: (15.0, 46.0) 40×40    slot 7: (115.0, 46.0) 60×40
//   slot 2: (75.0, 46.0) 40×40    slot 3: (75.0, 89.0) 40×60
//   slot 4: (75.0, 152.0) 40×40   slot 0: (15.0, 89.0) 40×60
//   slot 1: (134.0, 89.0) 40×60   slot 5: (15.0, 152.0) 40×40
//   slot 6: (134.0, 152.0) 40×40  slot 9: (55.0, 89.0) 20×20
//   slot10: (55.0, 152.0) 20×20   slot11: (115.0, 152.0) 20×20
// Anti-tamper hash table blocks (MAIN_HASH_CLASS) interspersed — skipped.
// 2026-09-17: antes esta funcion dibujaba el item y la logica la hacia un
// hit-test inventado en HUD_Pass6 (InventoryEquipmentHitTest), sin la
// validacion ni el color de la casilla.
```

### Línea 1807 — antes de `// Player input helpers`

```cpp
// FUN_004d23b0 @ 0x004D23B0 — Inventory grid render + click dispatcher.
// 2026-05-08: port FIEL completo movido a `Item/Item_ClickHandler.cpp`
// (~600 líneas). Antes era stub vacío bloqueando toda la cadena de
// interacción con items (pickup, drop, sell, use, hotkey).
//
// La signatura real (per IDA `004D23B0_sub_4D23B0.c`) es:
//   void __cdecl FUN_004d23b0(char* origin_x, int origin_y, short* inv_base,
//                             int grid_w, int grid_h, char mode_flag);
//
// Caller sites en este archivo (SecondPassword.cpp:712-782) ya pasan los
// argumentos correctos como esa signatura — el stub anterior tenía una
// signatura errónea pero la convención de llamada __cdecl + tamaños
// compatibles hicieron que linkeara sin warning.
```

### Línea 1860 en `SetPlayerStop` — antes de `auto ItemTwoHand = [](int type) -> unsigned char {`

```cpp
    // Helper para leer el byte TwoHand de ItemAttribute[type-399].
    // IDA: `*((_BYTE *)&ItemAttribute[v5 - 399] - 34)` → struct stride 0x40 con
    // un offset peculiar (-34 desde el inicio del elemento). Mu structures.h
    // muestra TwoHand en offset +30 dentro de ITEM_ATTRIBUTE. La expresión
    // `&arr[i] - 34` con sizeof=64 == (i-1)*64 + 30, o sea TwoHand del item
    // anterior. Mantenemos el cálculo idéntico al IDA para preservar semántica.
    // 2026-08-08 CRASH-FIX (0xC0000005 al cerrar el inventario con V):
    // stack = Game_CharSelectTick → MoveMonsterClient → SetPlayerStop → este lambda,
    // leyendo `[base + 0x1A00 - 0x21]` con base ≈ 0 (log: addr=0x005D4E95,
    // param1=0x000019DF, eax=0x1A00, edi=0).
    // Dos agujeros: (a) el guard sólo miraba `== 0`, pero DAT_07d78068 se
    // corrompe a valores CHICOS (ver el watchdog de ItemAttribute en
    // Render_Frame.cpp / Item_GetAttribute), y (b) `type` llega como 0xFFFF
    // cuando la mano está vacía — justo lo que pasa al desequiparse todo — y
    // `(0xFFFF-399)*64` indexa ~4 MB después de la tabla.
    // Validamos base y rango igual que el resto de los accesos a ItemAttribute.
```

### Línea 2004 — antes de `void __cdecl SetPlayerWalk(int param_1) {`

```cpp
//
// 2026-05-04: REEMPLAZA stub que devolvía hardcoded `uVar8 = 2` para todo
// non-DarkLord. Por eso el player caminaba sin animación (action=2 es la
// pose idle del modelo). Port faithful from IDA L62-227.
```

### Línea 2025 en `SetPlayerWalk` — antes de `char bSafeZone0 = *(char *)(param_1 + 0x34e);`

```cpp
    // +0x34E (=846) es **SafeZone**, NO dead (el dead real es +0x2FD).
    // Ver CLAUDE.md 2026-08-10; el nombre viejo `dead` mentía.
```

### Línea 2113 en `SetPlayerWalk` — antes de `auto ItemTwoHand = [](short type) -> unsigned char {`

```cpp
        // Tiene armas, está vivo, no es Atlans, no está exhausto:
        // 2026-05-07: TwoHand approximation reemplazado por lookup REAL en
        // ItemAttribute table (DAT_07d78068, stride 64). IDA hace
        // *((BYTE*)&ItemAttribute[N-399] - 34) — stride 64, byte-offset 30
        // dentro del item anterior. Mantener idéntico para preservar semántica.
```

### Línea 2233 en `SetPlayerWalk` — antes de `int v19 = rand() % 2;`

```cpp
            // BUG-FIX 2026-08-18: el indice de la tabla de sonidos era el action ID
            // actual del entity (+0x105 anim_state). IDA 0x443930 L288-289 es:
            //     v19 = rand() % 2;
            //     PlayBuffer(*(short*)(Models + 2*(v19 + 94*type) + 170) + 170, c, 0);
            // o sea la tabla tiene SOLO 2 entradas (los dos sonidos de paso del
            // modelo) y se elige una al azar. Con el action ID (13..33 al caminar)
            // el indice se iba muy lejos de esas 2 entradas y leia campos ajenos
            // del struct de modelo -> ids basura: 136 -> mBaliAttack2 (sonido de
            // ATAQUE sonando al caminar) y 104 -> slot 274, que ni existe.
```

### Línea 2257 en `MoveCharacterPosition` — antes de `float out[3] = {0.0f, 0.0f, 0.0f};`

```cpp
    // PORT FIX: el mismo artefacto de float[3] partido por Ghidra que en Terrain_Light Entity_GetLightScale.
    // local_3c/local_38/local_34 eran el buffer de salida contiguo de 3 floats que
    // esperaba Vector_Rotate, pero MSVC no garantiza el layout de los locales.
```

### Línea 2284 — antes de `void __cdecl SetCharacterClass(int c) {`

```cpp
// SetCharacterClass @ 0x0045C130 — SetCharacterClass(entity)
//
// IDA-ported 2026-04-26 (audit #5). Antes era un stub parcial mal-llamado
// "Entity_CancelTarget" que sólo copiaba el cluster primario con offsets
// incorrectos. Reescrito completo per `0045C130_SetCharacterClass.c`:
//
//   1. Sólo opera sobre entity_type == 390 (= 0x186, local player).
//   2. Copia cluster primario (4 slots: 624/648/672/696) con +400 offset.
//   3. Si entity[+847] != 0 (caso "ya seteado"): early return.
//   4. Si entity[+847] == 0: copia cluster secundario (504/528/552/576/600)
//      con fallback formula `(skin&7) + 4*(skin>>3) + base` cuando el slot
//      del CharData es -1.
//   5. SetCharacterScale + sub_47E3C0 al final del else branch.
//
// Hashtable obfuscation y comparación con `Hero` global: omitidos (anti-tamper
// noise / no requeridos para gameplay observable).
// IDA: SetCharacterClass (0x0045C130)
```

### Línea 2312 en `SetCharacterClass` — antes de `const short prevHelper = *(short*)(c + 696);`

```cpp
    // 2026-08-24: valor previo del helper, para detectar el CAMBIO abajo.
```

### Línea 2319 en `SetCharacterClass` — antes de `{`

```cpp
    // ── Spawn/borrado del pet del HEROE (Guardian Angel y monturas) ─────────
    // El pet NO se dibuja desde RenderCharacter: `ChangeCharacterExt`
    // (0x45C8C0 L73-88) lo crea como entidad del pool de bugs —
    //     *(WORD*)(c + 696) = Type + 816;
    //     CreateBug(816 | 195 | 267, c + 16, c, 0, 0);
    // — y de ahi lo tickea MoveBugs y lo dibuja RenderBugs. Por eso
    // RenderCharacter solo tiene rama para el Imp (817) y ninguna para el 816.
    //
    // Pero `ChangeCharacterExt` se llama SOLO con el CharSet que llega por red:
    // char-select (ReceiveCharacterList L68), otros jugadores del viewport
    // (Combat_PacketDispatch L317) y el F3/13 (ProtocolCore L1629). El equipo
    // del HEROE in-world no viene por ahi sino de CharacterMachine, o sea pasa
    // por esta funcion — que setea el tipo pero nunca creaba el bug. Medido con
    // sonda: todos los `CreateBug type=816` salian con `isHero=0`, y el pet
    // propio no se dibujaba nunca (el de otros jugadores si).
    //
    // DESVIACION documentada: IDA no crea el bug en SetCharacterClass. No
    // encontre el camino por el que el original se lo da al heroe; puede estar
    // en el arrastre del pool desde char-select. Replicamos aca el par
    // DeleteBug+CreateBug de ChangeCharacterExt, gateado al CAMBIO de tipo para
    // no re-spawnear en cada llamada (esta funcion corre al cambiar el equipo,
    // no por frame).
```

### Línea 2423 — antes de `int __cdecl CalculateAll(int characterMachine, int /*p2*/, int /*p3*/) {`

```cpp
// sub_47E3C0 @ 0x0047E3C0 (293 bytes) — CharData_RecalcStats wrapper.
// Port FIEL desde IDA decompile (2026-05-02). Calls 8 stat helpers in order
// then computes derived stats:
//   - this[+1396] = this[+1388] - this[+1378]   (rango de stat máx - mín)
//   - this[+1398] = this[+1390] - this[+1378]
//   - this[+1400] = this[+1374] + rand%(1376-1374+1) - this[+78]  (tirada de daño aleatorio)
//   - this[+1402] = this[+58]  - this[+1384] (clampeado a 100)  (% de crítico)
//   - this[+1404] = this[+76]  - this[+1382] (clampeado a 100)  (% de esquive)
//   - this[+1406] = (rand()%100 < this[+1402]) ? 1 : 0  (tirada de crítico)
//   - this[+1407] = (rand()%100 < this[+1404]) ? 1 : 0  (tirada de esquive)
// IDA: FUN_0047E3C0 (0x0047E3C0)
```

### Línea 2475 — antes de `void __cdecl CheckGate(void)`

```cpp
// CheckGate @ 0x004AC140.
// GateAttribute: 100 registros de 9 bytes (activo, mapa, minX, minY, maxX, maxY,
// gate destino, direccion, nivel minimo).  Si el heroe pisa el rectangulo pide
// al server el gate con C1:06:1C:<gate>:00:00.
//
// Detalles fieles a IDA que el port anterior no tenia:
//  - el recorrido NO corta al encontrar un gate: cada rama termina en
//    LABEL_121 y sigue con el siguiente registro;
//  - `LoadingWorld = 50` al rechazar por nivel (antirrebote del aviso) y
//    `LoadingWorld = 9999999` (`(int)&unk_98967F`) al pedir el viaje, que
//    ReceiveTeleport baja a 30; si el pedido se descarta por el antirrebote
//    vuelve a 0;
//  - los avisos van con etiqueta vacia (byte_7E11E50.. son BSS sin escritor),
//    no "ERROR".
// IDA: CheckGate (0x004AC140)
```

### Línea 2580 — antes de `// IDA: TERRAIN_INDEX (0x004F6C30)`

```cpp
// Combat_SendMovePathPacket (Send_MovePacket), Combat_DispatchHeroSkillAttack (Attack), Combat_CheckArrowRequirement (CheckArrow),
// Combat_UseElfSkill (UseSkillElf), Action (Action big switch),
// movidos a src/Combat/Combat.cpp
// (B3 refactor 2026-05-07, 1216 lines).
```

### Línea 2587 — antes de `void __cdecl RenderTerrain(char EditFlag) {`

```cpp
// IDA: RenderTerrain (0x004F9AC0)
// Reemplaza la fallback flat-shaded previa por el decompile fiel de IDA (352 b).
// Mantiene flujo y orden de Render States del binario:
//   sub_4F98C0 (terrain light setup) → WaterMove update → [Edit: SelectFlag=0 +
//   Map_InitRayCast | Run: DisableAlphaBlend] → TerrainFlag=0 →
//   RenderTerrainFrustrum(EditFlag) → [Edit: si SelectFlag, RenderTerrainTile del
//   tile pickeado | Run: EnableAlphaTest(1) → overlay pass TerrainFlag=2 →
//   ambient objects → DisableDepthTest/EnableCullFace/AlphaBitmaps/EnableDepthTest]
//   → toggle ^=1 → sub_4F9A30.
//
// Dependencias verificadas en IDA (bytes de operando):
//   Hero=0x07abf5d8, World(0x0055a7ac), WorldTime=0x05826e08,
//   WaterMove=0x07eeb214, SelectFlag=0x07eab1fc, SelectXF/YF=0x080ab288/28c,
//   TerrainFlag=0x0838bc44, toggle=0x0839bc88, unk_55A76C=0x0055a76c.
//   - WorldTime: en el binario es float ((float)timeGetTime() en CalcFPS 0x43FD70);
//     en nuestro codebase es int g_AnimTick y todos sus readers lo usan como int.
//     2026-09-03: se lee `(long long)WorldTime % N`, NO `(int)`.  `timeGetTime()`
//     pasa de 2^31 ms a las ~24.8 dias de uptime y ahi el cast a int satura, con
//     lo que la animacion de agua queda congelada.  IDA usa `(__int64)WorldTime`
//     en todos sus sitios justamente por eso.
//   - unk_55A76C: único xref es el read de abajo (sin writer en el binario) → el
//     2º pass overlay (TerrainFlag=2) es inerte también en el original.
//   - Callees aún fallback (a portar en esta cadena): RenderTerrainFrustrum
//     (#2, 0x004F97E0), RenderTerrainTile RenderTerrainTile (#3, 0x004F8480).
```

### Línea 2805 — antes de `extern "C" bool __cdecl CharacterAnimation(int c, int o);`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// IDA: MoveCharacter (0x00449900)
// ─────────────────────────────────────────────────────────────────────────────
// Tick de entidad por frame. Decompile de IDA: 2773 líneas / 32793 bytes; ~70% es
// anti-tamper hash-table noise (dword_55C9BC8/BCC/BD0/BD4 + sub_403F80/04280/
// 04330/0423710/0404400 ref-count + XOR encryption around CharacterMachine
// lecturas). Per la política del proyecto (CLAUDE.md) todas las operaciones de hash-table se saltean
// — son ofuscación, no lógica de juego.
//
// El llamador pasa la misma entidad como `c` y como `o` (MoveCharacterClient → cc, cc).
// Mapped: `c == o == ent`. Behavior:
//   1. Sync entity world pos+rot → Models[entType] render slot
//   2. Hero-only: decrement attack/magic speed buff timers (CA+42/44),
//      recalc stats when timer expires
//   3. Stationary NPC tagging (event NPCs 206..208 in cities)
//   4. Knockback/tambaleo (c+772) y deslizamiento al objetivo (c+773)
//   5. Pickup-bag bouncing physics (entType=236)
//   6. Death dissolve (c+765 ragdoll)
//   7. CharacterAnimation() drives idle action selection
//   8. Attack swing (c+757) → AttackStage / AttackEffect
//   9. Bone particle effect spawn (o+404)
//  10. Skill effect dispatch from c+770 queue (cases 5/8/9/10/12/13/14/30..36/
//      41/42/48/49/55/56) → CreateEffect/CreateJoint/PlayBuffer
//  11. Targeted-skill effects when c+784 != -1 (cases 1/2/4/7/11/16/17/24/26/27/
//      28/51/52)
//  12. Joint chain rendering (c+816 ∈ 1253/1254/1256/1260) — sword/spear trail
//  13. Weapon blur trail (CreateBlur) when AnimFrame >= 3.0
//  14. Pickup-bag splash (entType=236)
//
// Original signature: `void __cdecl MoveCharacter(DWORD c, DWORD o)`. Our ABI
// es `void __cdecl MoveCharacter(int p1)`; los dos argumentos son la misma entidad.
//
// Dependencias (todas ya en el árbol como FUN_xxxxxxxx):
//   Stats_CalcBase   Stats_CalcBase (sub_47D410)
//   Stats_CalcMagicDmgRange   Stats_CalcMagicDmgRange (sub_47DAE0)
//   CalculateAttackSpeed   CHARACTER_MACHINE::CalculateAttackSpeed
//   RequestTerrainHeight   RequestTerrainHeight
//   Particle_Spawn   Particle_Spawn
//   CharacterAnimation
//   SetPlayerStop   SetPlayerStop
//   SetAction   SetAction
//   CreateChat   CreateChat
//   CreateBlood
//   DeleteCloth   DeleteCloth
//   AttackStage
//   AttackEffect   AttackEffect (existing port at line 3083)
//   CreateEffect   CreateEffect
//   Joint_Create   CreateJoint
//   Effect_SpawnSmokeBurst   CreateBomb
//   BMD_TransformPosition   BMD::TransformPosition
//   BMD_Animation   BMD::Animation
//   AngleMatrix    (no FUN_)
//   VectorRotate   = Vector_InverseRotate
//   FUN_004b1170   FindHotKey
//   CreateArrows
//   Math_Fabs   fabs
//   FUN_0046fe40   Joint_Find
//   FUN_004451c0   AngleVectorOffset
//   Effect_SpawnBombRing   bomb-ring effect
//   FUN_0046c5a0   skill impact particles
//   FUN_0046c7f0   directional blood
//   FUN_0045fae0   lectura hash de 1 byte (lectura de la cola de skills)
//   Alpha   Alpha
//   PlayBuffer   PlayBuffer
//   SetPlayerDie / DeleteJoint / CreateBlur — local helpers below
//
// Anti-tamper SKIPPED everywhere — all `if (c == Hero) { hash-decrypt; ...
// hash-encrypt; }` colapsan a una lectura directa de los campos de CharacterMachine.
//
// Declaraciones externas locales a esta unidad de traducción:
```

### Línea 2896 — antes de `extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType);`

```cpp
// 2026-08-24 FIX (Soul Barrier: arcos gruesos y "dobles"): esto era un NO-OP,
// con el comentario "no-op until joint pool wired" — pero el pool esta cableado
// desde 2026-05-08, cuando se porto `DeleteJoint` (0x0046FE00). Quedo el stub
// local y `MoveCharacter` siguio llamandolo, o sea el borrado nunca ocurria.
//
// Efecto medido: el skill 16 spawnea 5 joints 266 con Scale 20 y ANTES hace
// `DeleteJoint(266, Owner, 0)` para sacar los que ya hubiera. Los de Scale 50
// que deja `InsertBuffPhysicalEffect` (0x43BDE0, el camino de viewport cuando la
// entidad aparece con el buff ya activo) nunca se borraban, asi que convivian
// los dos grupos: censo del pool `quads20=3190 quads50=3190`, exactamente 50/50,
// ~10 joints donde el original tiene 5. De ahi que los arcos se vieran mas
// gruesos y cargados que en el cliente original.
```

### Línea 2965 en `mc_JointFind` — antes de `const int stride = 0x9d8;`

```cpp
    //
    // 2026-09-04: antes era un stub `return 0` con el comentario "el pool no
    // esta alocado".  Eso quedo viejo -- DAT_07b27150 esta dimensionado desde
    // 2026-05-08.  Con el stub, MoveCharacter case 27 (Greater Defense) creaba
    // 5 joints nuevos cada vez que le re-aplicaban el buff en vez de reusar los
    // que ya estaban girando.
```

### Línea 2991 — antes de `void __cdecl SetPlayerDie(int c_in)`

```cpp
// SetPlayerDie @ 0x00444D90 — SetPlayerDie (1057 bytes IDA, port FIEL 2026-05-07).
// Real signature: void __cdecl SetPlayerDie(DWORD c).
// (functions.h declaró SetPlayerDie como "Entity_TeleportEnd" — eso es un
// mismap del port-time. La función AT 0x00444D90 ES SetPlayerDie per IDA.)
//
// Differentiation:
//   - Player (type 390), normal class (NOT 206-208): SetAction(c, 131)  ← death anim
//   - Player class 206-208 (special): explosion FX (210 + 211×10), c[0]=0
//   - NPC type 295 (=v13==0): explosion FX (226+227)×8, c[0]=0
//   - NPC type 300 (=v13==5): explosion FX (210 + 211×10), c[0]=0
//   - Other NPCs: SetAction(c, 6)  ← death anim
// El ruido de hash table (camino sólo-Hero, L32-119) se saltea per la política del proyecto.
// IDA: SetPlayerDie (0x00444D90)
```

### Línea 3079 — antes de `static inline void mc_SetPlayerDie(DWORD c)`

```cpp
// mc_SetPlayerDie — wrapper que usa la lógica de envejecimiento del ragdoll de MoveCharacter.
// El SetPlayerDie de IDA NO toca dead_flag (lo hace el llamador, ReceiveDie).
// Acá agregamos los efectos secundarios del flag porque el camino del ragdoll de MoveCharacter
// espera que la entidad quede marcada como muerta después de esta llamada.
```

### Línea 3087 en `mc_SetPlayerDie` — antes de `}`

```cpp
    // 2026-09-02: REMOVIDA la escritura `*(char*)(c + 0x2FD) = 1;`.  Era una
    // invencion del port: SetPlayerDie (0x00444D90, 1057 bytes) no toca +765 en
    // ninguna de sus lineas — el unico writer del dead_flag es ReceiveDie.
    // El bloque que llama aca (LABEL_195, IDA L796-806) usa +765 como CONTADOR:
    //     if ( *(_BYTE *)(c + 765) )
    //         if ( (unsigned __int8)++*(_BYTE *)(c + 765) >= 0xFu )  SetPlayerDie(c);
    // Al re-escribir 1 desde el wrapper, el contador nunca podia pasar de 15 y
    // el flag quedaba clavado en != 0 para siempre.  Eso importa porque +765 es
    // el filtro de "vivo" del barrido de sub_45FEC0 (IDA L168 `!v16[18]`), que es
    // quien reporta los blancos al server con el 0x1D.
    // 2026-07-27 FIX (alas rojas "PK"): NO setear dead_flag (0x34e) aquí. El
    // IDA SetPlayerDie NO lo toca — sólo ReceiveDie (el packet de muerte real)
    // lo setea. Este mc_SetPlayerDie lo llama el ragdoll-aging (c+765 counter);
    // si ese counter se activa espuriamente sobre el héroe VIVO, el dead_flag
    // quedaba stuck en 1 → el render (bDead = c+0x34e) lo trataba como muerto →
    // body light rojo (1.0,0.1,0.1) = el bug de "alas rojas PK" intermitente.
    // Confirmado por el diag HEROLIGHT (34e=1 con el pj vivo caminando).
    // Removidos también el clear de alive_flag (0x2EC) — sólo ReceiveDie maneja
    // la transición de estado de vida.
```

### Línea 3131 en `MoveCharacter` — antes de `float (* const BoneMatrix)[3][4] = (float (*)[3][4])g_BoneScratch;`

```cpp
    // La estela del arma usa el buffer GLOBAL de huesos (0x06970A9C), igual que
    // IDA: `BMD_Animation(v422, (float (*)[3][4])BoneMatrix, ...)` y despues
    // `BoneMatrix[3 * bone]`.  Antes esto era un `float[200][3][4]` LOCAL y sin
    // inicializar: los huesos que BMD_Animation no escribe quedaban con basura de
    // stack, asi que los dos extremos de la hoja salian en un punto fijo cualquiera
    // -- la estela aparecia pero despegada del arma.
```

### Línea 3584 en `MoveCharacter` — antes de `PlayBuffer(97, 0, 0);`

```cpp
            // L1664: seteo de flag sólo para el Hero (anti-tamper removido)
            // salteado — era una ronda de hash-encrypt sobre CharacterMachine
            // 2026-08-23: faltaba el sonido del skill (IDA L1817, justo despues
            // del bloque de hash-encrypt que se omite por policy).
```

### Línea 4015 en `MoveCharacter` — antes de `BMD_Animation((void*)v422, (int)BoneMatrix, AnimationFrame,`

```cpp
                    // 2026-09-26 FIX (no se veia la estela de la hoja): el 6to
                    // argumento de BMD_Animation es el Angle de la entidad, no un
                    // buffer de salida.  El port pasaba un array de CEROS, asi que
                    // los huesos se posaban con facing (0,0,0) en vez del real y los
                    // dos extremos de la hoja salian en una orientacion fija.
```

### Línea 4090 en `MoveCharacterVisual` — antes de `{`

```cpp
    // 2026-08-10 — SafeZone update por frame (IDA MoveCharacterVisual L614):
    //     *(BYTE*)(c + 846) = (TerrainWall[Terrain_Load(x, y)] & 1) == 1;
    // +0x34E (846) es **SafeZone**, NO dead_flag (el dead real es +0x2FD, ver
    // IDA ReceiveDie L18). Sin este write el flag quedaba pegado en su valor de
    // spawn: nunca se activaba la música de pueblo, ni el bind del arma a la
    // espalda, ni el gate de "no atacar en zona segura".
    //
    // Hex-Rays perdió los args de Terrain_Load acá (los slots de stack `x`/`yg`
    // los reusó el ruido de hash-table), así que usamos la posición de mundo
    // actual de la entidad → celda, que es lo que hace CreateCharacterPointer
    // en el spawn y la única lectura posible (la celda del personaje).
```

### Línea 4111 en `MoveCharacterVisual` — antes de `{`

```cpp
    // -- switch por tipo de entidad (IDA MoveCharacterVisual L764) --------
    // Efectos ambientales por NPC/monstruo.  2026-09-02: los **31** cases del
    // binario estan portados (0x10E 0x110 0x111 0x113 0x114 0x119 0x11A 0x11B
    // 0x11D 0x122 0x128 0x129 0x12B 0x12E 0x12F 0x133 0x135 0x137 0x138 0x139
    // 0x13A 0x13B 0x13E 0x141 0x142 0x145 0x152 0x157 0x15C 0x179 0x186).  El
    // comentario viejo decia que solo estaba el del herrero y quedo obsoleto.
    //
    // Lo unico del cuerpo que NO se porto es el bloque `if (c[836] > 0)` de
    // IDA L713-754 (los tipos de aura 1251 y 1252): **es codigo muerto en el
    // binario**.  Las dos ramas calculan una posicion (`v162` en la 1251, y el
    // `WorldPosition` de TransformPosition en la 1252) y despues no la usan --
    // no hay ningun Particle_Spawn ni CreateEffect detras, y L759 hace memset
    // de WorldPosition acto seguido.  Portarlo seria copiar un no-op.
    //
    // Luminosidad con flicker, común a todo el switch (IDA L761-763):
    //     v63 = (rand() % 8 + 2) * 0.1     → 0.2 .. 0.9
```

### Línea 4704 — antes de `void __cdecl CreateFrustrum2D(float *param_1)`

```cpp
// IDA: CreateFrustrum2D (0x004F8EB0)
// CORRECCIÓN 2026-05-04: la decompilación previa de Ghidra confundió los
// nombres. GetScreenWidth NO es un frame counter — es GetScreenWidth(). Los
// constants `_DAT_00552cbc=1190.0` y `_DAT_00552cb8=540.0` son las half-widths
// de la frustum quad (en view-space units), NO velocidades de rotación.
// _DAT_0055283c = 1/640 (= screen pixel→aspect ratio).
//
// Construye un quad 2D top-view del frustum proyectado: rota por Z=45° las 4
// corners hardcoded, las traslada por param_1 (cam pos), y las escribe a
// FrustrumX/218 (X/Y respectivamente) en tile coords (×0.01 = ÷100).
//
// Esta versión SÍ funciona para in-game. La duplicada Camera_SetMatrix en
// src/Render/Camera.cpp es código muerto que opera sobre DAT_07eab1bc..1e8
// (corners ya transformadas por Camera_SetupFrustum a world coords), lo cual
// duplicaría la transformación si se invocara — no se llama desde ningún
// lado y no debe wirearse.
```

### Línea 4752 — antes de `int __cdecl FUN_004f98c0(int a1, int a2, int a3, int a4, int a5)`

```cpp
// === FUN_004f98c0 — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
```
