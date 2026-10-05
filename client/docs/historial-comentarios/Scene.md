# Historial de comentarios: `src/Scene/`

Comentarios de desarrollo movidos desde `src/Scene/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Scene/Scene_AssetLoad.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

## `src/Scene/Scene_CharSelect.cpp`

### Línea 214 en `Scene_CharSelect` — antes de `glClearColor(0.0f, 0.0f, 0.0f, 1.0f);`

```cpp
        // BUG-FIX: 0x3f800000 son los bits de 1.0f, no la magnitud.
```

### Línea 330 en `Scene_CharSelect` — antes de `float posBuf[3];`

```cpp
                    // ── BUG-FIX: fStack_5c/uStack_58/fStack_54 son 3 variables LOCALES
                    // separadas. MSVC no garantiza que estén contiguas en stack, así que
                    // pasar &fStack_5c a Camera_ProjectWorldToScreen (que lee [0]/[1]/[2]) producía
                    // proyecciones erráticas (TPos[2] no matcheaba con M*input).
                    // Usamos un array contiguo posBuf[3] para garantizar layout.
```

### Línea 353 en `Scene_CharSelect` — antes de `int nameProjY = 0;`

```cpp
                    // ── BUG-FIX: local_70 está declarado float (línea 202) pero
                    // Camera_ProjectWorldToScreen escribe un int en él vía cast pointer. La lectura
                    // posterior `(int)local_70` hace conversión FPU float→int sobre
                    // el bit-pattern denormal, dando ~0 y poniendo los nombres en
                    // y=-15. Usamos un int local separado para la proyección.
```

### Línea 486 en `Scene_CharSelect` — antes de `glColor4f(0.0f, 0.0f, 0.0f, 0.8f);`

```cpp
                // BUG-FIX: 0x3f4ccccd = 0.8f bits, pasado como int → 1062836429.0f.
```

### Línea 498 en `Scene_CharSelect` — antes de `glColor4f(0.0f, 0.0f, 0.0f, 0.8f);`

```cpp
                // BUG-FIX 2026-07-17: la Y del overlay de "flecha derecha bloqueada" era
                // 0.0f (se dibujaba fuera de la flecha) → la flecha no se veía bloqueada
                // al llegar al máximo de clase disponible (ej: Fairy Elf sin poder ir a MG).
                // IDA RenderColor(384, dialogY+196, ...) → usa la misma Y que la izquierda.
```

### Línea 540 en `Scene_CharSelect` — antes de `if ((int)DAT_005616b0 >= 0)`

```cpp
            // BUG-FIX 2026-07-17: DAT_005616b0 es DWORD (unsigned); `-1 < DAT_005616b0`
            // convertía -1 a 0xFFFFFFFF → comparación SIEMPRE falsa → RenderInputText
            // (el campo del nombre) nunca se dibujaba. IDA: `if (dword_5616B0 >= 0)`.
```

### Línea 561 en `Scene_CharSelect` — antes de `{`

```cpp
                // BUG-FIX 2026-07-17: la entrada in1={-8,-800,79} y la salida de
                // VectorIRotate estaban en locals SEPARADOS no contiguos (fStack_5c/
                // uStack_58/fStack_54 y tStack_68.cx/cy + fStack_60), y la salida se leía
                // con (float)cast (convert) en vez de reinterpret de los bits float →
                // CameraPosition del preview quedaba en una posición basura → el char se
                // renderizaba fuera del viewport (recuadro vacío). Arrays contiguos + IDA-fiel.
```

### Línea 577 en `Scene_CharSelect` — antes de `_DAT_07abf06c = 0.0f; _DAT_07abf070 = 5.0f; _DAT_07abf05c = 1.0f;`

```cpp
                // BUG-FIX 2026-04-20: _DAT_07abf0?? están tipados `float`;
                // asignar 0x40a00000 / 0x3f800000 hace int→float (1e9), no 5.0f / 1.0f
```

### Línea 607 en `Scene_CharSelect` — antes de `float fR, fG, fB;`

```cpp
            // Create button brightness
            // BUG-FIX (2026-04-21): uVar* are uint holding bit-patterns 0x3f800000 (1.0f)
            // / 0x3f000000 (0.5f). glColor3f expects GLfloat → int→float conv gives
            // 1065353216.0f / 1056964608.0f → OpenGL clamps to 1.0 → always white
            // regardless of empty-slot state. Use float literals.
```

### Línea 621 en `Scene_CharSelect` — antes de `if (bVar10 && ((0x11d < DAT_083a427c) && (DAT_083a427c < 0x164)) &&`

```cpp
            // BUG-FIX 2026-04-26: el hover-text "NEW CHARACTER" (sprite 0x11) se
            // mostraba aunque la cuenta tuviera los 5 slots ocupados. El brightness
            // del botón base (sprite 0x10) ya gateaba en `bVar10` (hay slot vacío),
            // pero el render del label hover faltaba el mismo guard. Original solo
            // muestra hover si quedaba algún slot libre.
```

### Línea 633 en `Scene_CharSelect` — antes de `if (bVar10) { fR = fG = fB = 0.5f; }`

```cpp
            // Delete char button brightness (same BUG-FIX as above — bit-pattern→float conv)
```

## `src/Scene/Scene_CharSelect_Nav.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Scene_CharSelect_Nav.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 13751-14826 (1076 lines).
//
// Char-select slot navigation helpers:
//   CSQuest_FindQuestContext (IDA: FUN_004011D0)
//   CSQuest_CheckActCondition (IDA: FUN_00401650)
//   CSQuest_ShowDialogText (IDA: FUN_004017E0)
//   CSQuest_clearQuest (slot-list navigation forward)
//   FUN_00401af0 (slot-list navigation back)
//   ... and related slot scroll/select helpers.
//
// Manejan el scroll del panel de char-select / lista de clases y la elección de slot.
```

### Línea 22 — antes de `int __cdecl LevelConvert(BYTE Level);`

```cpp
// -- Declaraciones de funciones que viven en otros modulos --------------
// Agregadas por el refactor B3: se declaraban localmente en el archivo del
// que se movieron estas funciones. Migrar a functions.h mas adelante.
```

### Línea 44 — antes de `#ifndef LODWORD`

```cpp
// IDA Hex-Rays intrinsic shims (mirror of stubs.cpp shims).
```

### Línea 223 — antes de `double __cdecl Math_Fmin(float a1, float a2)`

```cpp
// Math_Fmin @ 0x00512A10 — Math_Fmin(a, b) → min(a,b)
// Math_Fmin (IDA-activated, was Ghidra stub)
```

### Línea 238 — antes de `double __cdecl Math_Fmax(float a1, float a2)`

```cpp
// Math_Fmax @ 0x00512A30 — Math_Fmax(a, b) → max(a,b)
// Math_Fmax (IDA-activated, was Ghidra stub)
```

### Línea 326 — antes de `char __cdecl HotkeyBar_FindFreeSlot(char *_this)`

```cpp
// HotkeyBar_FindFreeSlot @ 0x0047E3A0 — HotkeyBar_FindFreeSlot
// Scans 12 hotkey bar slots (stride 0x44) starting at CharData+0x232.
// Returns slot index with low byte = 1 on success, or raw index (no flag) if full.
// HotkeyBar_FindFreeSlot (IDA-activated, was Ghidra stub)
```

### Línea 347 — antes de `int __cdecl Character_FindByKey_WithClear(int a1)`

```cpp
// Character_FindByKey_WithClear @ 0x0045ACC0 — Character_FindByKey_WithClear
// Scans entity array (stride 0x394, 400 entries) for an entity whose
// la clave (short en +0x1dc) coincide con param_1. Limpia el byte +0x2e8 de cada
// entidad durante el escaneo. Devuelve el índice coincidente, o 400 si no lo encontró.
// Character_FindByKey_WithClear (IDA-activated, was Ghidra stub)
```

### Línea 376 — antes de `void __cdecl Characters_SetActionAll(int Action)`

```cpp
// Characters_SetActionAll @ 0x0045AD10 — Characters_SetActionAll
// Setea la acción dada en todas las entidades vivas de tipo DK (0x186).
// Resetea el Angle a (0, 0, 180°) antes de aplicar la acción.
// Characters_SetActionAll (IDA-activated, was Ghidra stub)
```

### Línea 404 — antes de `void Characters_FreeAllBMDBuffers()`

```cpp
// Characters_FreeAllBMDBuffers @ 0x0045AD60 — Characters_FreeAllBMDBuffers
// Libera los buffers de heap de BMD por entidad (puntero en entity+0x114) de todo el array,
// y después libera el buffer BMD extra compartido (DAT_07abf164).
// Characters_FreeAllBMDBuffers (IDA-activated, was Ghidra stub)
```

### Línea 481 — antes de `char Sound_PlayFootstep()`

```cpp
// Sound_PlayFootstep @ 0x00451A90 — Sound_PlayFootstep
// Plays a terrain-appropriate footstep sound.
//   World 2 (Lost Tower): tile < 10 and != 3 → snd 10
//   World 0 or 3 (Lorencia/Devias): tile == 0 → snd 9
//   World 7 (Devil Square), alive → snd 11
//   Default → snd 8
// Sound_PlayFootstep (IDA-activated, was Ghidra stub)
```

### Línea 676 — antes de `void __cdecl Effect_PhysicsTick(DWORD Object)`

```cpp
// Effect_PhysicsTick @ 0x0046CA00 — Effect_PhysicsTick
// Actualización física por frame de una partícula de efecto a nivel del piso (p.ej. una moneda o un drop).
// Si el héroe está atacando y el efecto está en rango, lo atrae hacia
// el héroe con una velocidad proporcional al delta y un factor de fricción que decae.
// Effect_PhysicsTick (IDA-activated, was Ghidra stub)
```

### Línea 735 — antes de `int __cdecl FloatingLabel_Add(int a1, DWORD *a2, int a3, DWORD *a4, int a5)`

```cpp
// FloatingLabel_Add @ 0x004793F0 — FloatingLabel_Add
// Agrega al pool una etiqueta flotante de daño/curación (base DAT_07c82cd0, stride 0x70,
// hasta el límite del pool en 0x7c8588f). Elige el slot con la "edad" más chica
// value. Special range 0x4b5–0x4d8 gets a randomised lifetime (50+rand%32).
// param_1 = label text or ID, param_2 = world position (float[3]),
// param_3 = color/type, param_4 = screen offset (float[3]), param_5 = extra data.
// FloatingLabel_Add (IDA-activated, was Ghidra stub)
```

### Línea 847 en `SkillAttribute_SaveBin` — antes de `if (++nRowGT >= 1000) break;`

```cpp
        // 2026-09-08: el bound era `< 0x7d73104`, direccion absoluta del binario.
        // La base es `&SkillAttribute + 4` = 0x07D29D24 = GlobalText[0], y
        // (0x7D73104 - 0x7D29D24) / 300 = 1000 -- las 1000 filas de GlobalText.
        // (Confirma que esta funcion escribe GlobalText, no SkillAttribute.)
```

### Línea 867 en `Terrain_SpawnAmbientObjects` — antes de `BYTE *pbVar1 = (BYTE *)&DAT_081cb2ed;`

```cpp
    // BUG-FIX 2026-06-27: el while original usaba el bound de DIRECCIÓN ABSOLUTA
    // literal del binario fuente (136099341 = 0x081CB60D). En nuestro build
    // &DAT_081cb2ed vive en otra dirección, así que el loop caminaba memoria
    // ajena hasta una página no mapeada → AV (crash @0x005762E6, addr 0x021FA007),
    // disparado al wirear sub_4F7060 dentro del port 1:1 de RenderTerrain.
    // IDA: base 0x081CB2ED, bound 0x081CB60D → (0x320)/8 = 100 iteraciones.
```

### Línea 882 — antes de `int __cdecl Terrain_WaterWaveUpdate(int a1)`

```cpp
// Terrain_WaterWaveUpdate @ 0x004F9A30 — Terrain_WaterWaveUpdate
// Simulación de olas de agua por frame: mezcla los valores de altura adyacentes del
// buffer de olas anterior en el buffer actual, para una grilla de terreno de 256×256.
// param_1 selects between the two ping-pong buffers (0 or 1).
// Terrain_WaterWaveUpdate (IDA-activated, was Ghidra stub)
```

### Línea 933 — antes de `int __cdecl Collision_PointInPolygon(float a1, float a2, float a3, int a4, int a5, int a6,`

```cpp
// Collision_PointInPolygon @ 0x00512A50 — Collision_PointInPolygon
// Tests whether a 3D point (param_1,param_2,param_3) lies inside the polygon
// formado por param_4 vértices (los punteros están en el array param_5..param_8).
// param_9 elige el plano de proyección (1=YZ, 2=XZ, 4=XY y las variantes en sentido horario).
// Umbral param_10: si es > _DAT_00552580 (0.0f), param_9 se corre 3 bits a la izquierda.
// Devuelve 1 si está adentro, 0 si está afuera.
// Collision_PointInPolygon (IDA-activated, was Ghidra stub)
```

### Línea 1142 — antes de `// IDA: FUN_00401010 @ 0x00401010 — calls quest table init`

```cpp
// ── FUN_00401010 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 10-byte: simple wrappers & field ops ────────────────────────────────────
```

### Línea 1148 — antes de `// FUN_00401020 @ 0x00401020 (12 bytes)`

```cpp
// ── FUN_00401020 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 12-byte: CRT atexit wrappers ────────────────────────────────────────────
// 2026-05-07: these registered shutdown callbacks at literal source-binary
// direcciones (0x004XXXXX). En nuestro build el linker coloca el código en
// offsets distintos, así que dispararlas con atexit crashearía al cerrar el programa.
// Since the registered targets were stubs/no-ops anyway, neuter the wrappers.
```

### Línea 1166 — antes de `void __fastcall CSQuest_ShowDialogText(int param_1)`

```cpp
// ── CSQuest_ShowDialogText — movida desde stubs_helpers.cpp (refactor B3) ──
```

### Línea 1172 en `CSQuest_ShowDialogText` — antes de `if (param_1 < 0 || param_1 >= DIALOG_SCRIPT_COUNT) return;   // guard de port`

```cpp
    // 2026-08-21: los accesos a la tabla de dialogos ahora van por la struct
    // DIALOG_SCRIPT (ver globals.h).  Antes eran cuatro globals escalares
    // sueltos indexados con aritmetica de puntero tipado -> lecturas fuera de
    // rango.  El de m_lpszText acertaba de casualidad: `DAT_07cf5608` es
    // DWORD* y `+ param_1 * 0x100` da los 0x400 bytes correctos.
```

### Línea 1213 en `CSQuest_ShowDialogText` — antes de `wsprintfA((LPSTR)local_48, s__d___s_005580b0, iVar3 + 1, GlobalText[609]);`

```cpp
        // Sin respuestas: el binario ofrece la de cerrar, GlobalText[609]
        // (disasm 0x4018B9: `push offset GlobalText+2C9Ah`, y 0x2C9AC/300 = 609).
        // 2026-08-21: el port usaba &DAT_07d566d0, que no sale de IDA.
```

### Línea 1231 — antes de `void __fastcall CSQuest_clearQuest(int);`

```cpp
// ── CSQuest_clearQuest — movida desde stubs_externs.cpp (refactor B3) ──
```

### Línea 1257 — antes de `static void Quest_SendState(void *pThis)`

```cpp
// ── FUN_00401af0 — movida desde stubs_misc_helpers.cpp (refactor B3) ──
// FUN_00401af0 @ 0x00401AF0 — CSQuest: click sobre las respuestas del dialogo.
// NO es char-select (el comentario anterior decia eso y era un mal-guess del
// port): hit-test sobre las lineas de respuesta del panel de quest, y despacho
// por m_iReturnForAnswer del dialogo activo.
//   1 = aceptar la quest  -> CheckRequestCondition(bLastCheck=1); si falla,
//                            muestra el dialogo de rechazo y no manda nada.
//   2 = cerrar            -> CSQuest::clearQuest
//   3 = continuar         -> manda el paquete sin verificar condiciones
// Todas las ramas convergen en LABEL_107 (PlayBuffer 28 + encadenar
// m_iLinkForAnswer si es > 0 y no hubo rechazo).
// Paquete cliente->server de quest.  IDA sub_401AF0 arma en los dos sitios
// (L152-199 y L399-446) exactamente el mismo buffer:
//     *(DWORD*)v103 = 0x01C10003   -> len=3, packet = C1 ?? A2
//     v103[4] = 0xA2               -> opcode
//     append  *(BYTE*)(this + 116858)   (indice de quest actual)
//     append  1
// = [C1][05][A2][questIndex][01].  El wrapper anti-tamper lo mete en un frame
// C3 (`local_914[0] = 0xC3`), que es lo que corresponde: HackPacketCheck.txt
// da Encrypt = 1 para el indice 162 (0xA2), o sea C3/C4 con serial.
// El server (Protocol.cpp case 0xA2 -> CGQuestStateRecv) sólo lee QuestIndex y
// avanza el estado él mismo; el byte 1 del final lo ignora.
//
// 2026-08-21: el port armaba `{0xC1,1,0,0xA2,0}` con un XOR a mano y lo mandaba
// por un sendPkt propio — ni el opcode quedaba en su lugar ni el indice de
// quest viajaba.  Ahora usa el sender estandar del proyecto.
```

### Línea 1378 — antes de `// ═══════════════════════════════════════════════════════════════════════════════`

```cpp
// ── Send_ActionRequest — movida desde stubs_game.cpp (refactor B3) ──
// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 16
// ═══════════════════════════════════════════════════════════════════════════════
```

### Línea 1443 — antes de `void FUN_0043de60(void) {}`

```cpp
// ── FUN_0043de60 — movida desde stubs_render_helpers.cpp (refactor B3) ──
```

### Línea 1448 — antes de `// CPacketQueue_PushPacket @ 0x0043DF90 (38 lines) — Net_EnqueuePacket: copies packet into`

```cpp
// ── CPacketQueue_PushPacket — movida desde stubs_linker.cpp (refactor B3) ──
// ═════════════════════════════════════════════════════════════════════════════
// Tanda 20 — stubs para el linker (cuerpos vacíos de funciones que se llaman pero todavía no están decompiladas)
// ═════════════════════════════════════════════════════════════════════════════
```

### Línea 1483 — antes de `float __cdecl CreateAngle(float x1, float y1, float x2, float y2)`

```cpp
// ── CreateAngle — movida desde stubs_externs.cpp (refactor B3) ──
// ── Missing function stubs (all LNK2019 unresolved externals) ─────────────────
```

### Línea 1523 — antes de `int __cdecl Angle_Clamp(int param_1, int param_2, int param_3) {`

```cpp
// ── Angle_Clamp — movida desde stubs_misc2.cpp (refactor B3) ──
```

### Línea 1557 — antes de `float __cdecl TurnAngle2(float a1, float a2, float a3)`

```cpp
// IDA: TurnAngle2 (0x0043E1B0)
// Avanza curAngle hacia tgtAngle a lo sumo 'step' grados, manejando la vuelta de 360.
// Devuelve tgtAngle directo si está dentro del rango de step; si no, curAngle +/- step.
//
// BUG-FIX 2026-04-26 (audit #1): el decomp IDA original tenía 5 flags FPU x87
// sin reconstruir (`v5/v7/v9/v12/v14`) → branches indefinidos. Reescrito con
// math estándar "smooth turn-toward with 360° wrap", preservando la semántica
// observable: snap si |delta| <= step, sino avanzar `step` grados por el camino
// más corto (con wrap 0/360 respetado).
```

### Línea 1593 — antes de `void __cdecl SetPlayerAttack(int c_entity, int /*type*/, int /*flag*/, int /*extra*/) {`

```cpp
// ── SetPlayerAttack — movida desde stubs_game.cpp (refactor B3) ──
// SetPlayerAttack @ 0x00444410 (1627 bytes) — port FIEL desde IDA (2026-05-02).
// Setea la animación de ataque + el sonido de la entidad según:
//   - Entity type (c+2): non-player (39/40/51/302/default) vs player (390)
//   - Para el jugador: helper (c+696)=818/819 → a distancia, si no las armas izquierda/derecha
//     (c+624 LH, c+648 RH) determine animation 34..89.
// Calls: SetAction, CreateEffect, PlayBuffer,
//   SetAttackSpeed. All implemented.
//
// functions.h declara 4 argumentos pero IDA usa sólo 1 (DWORD c). Los extra se ignoran.
// IDA: SetPlayerAttack (0x00444410)
```

### Línea 1791 — antes de `void __cdecl SetPlayerMagic(int param_1) {`

```cpp
// ── SetPlayerMagic — movida desde stubs_game.cpp (refactor B3) ──
```

### Línea 1832 — antes de `// IDA: SetPlayerShock (0x00444B60)`

```cpp
// ── SetPlayerShock — movida desde stubs_misc2.cpp (refactor B3) ──
// SetPlayerShock @ 0x00444B60 — SetPlayerShock(DWORD c, int Hit)
// Reproduce la reacción de "me pegaron" (anim 130 para el jugador, anim 5 para los monstruos) más
// a hit-grunt sound (PlayBuffer). Port FIEL desde IDA decompile (546 bytes).
//
// 2026-05-08: portada como parte de la importación de bugfixes del DLL companion. El parche
// `IgnoreRandomStuck` (Patchs.cpp:291) saltea el *shock aleatorio del 50%* que se tira en
// ReceiveAttackDamage cuando la entidad es el jugador (tipo 390) — ése es un
// arreglo de gameplay, no de esta función. Los llamadores que quieran el comportamiento
// IgnoreRandom deberían gatear con `*(WORD*)(c+2) != 390` antes de llamar.
//
// Entity offsets:
//   +2    short  entity_type        (390 = player)
//   +4    int    sub_class
//   +0x105 byte  current_action     (62/63 = currently dying anim → skip)
//   +0x10 float[3] world position
//   +0x1C float[3] world angle
//   +0x1BC byte   move_type_flags   (& 7 == 2 = swimming)
//   +0x2B8 short  weapon_type       (818/819 = certain mounts that skip shock)
//   +0x2EC byte   alive_flag        (se limpia en el shock del jugador, per IDA)
//   +0x2FD byte   dead_flag         (set ⇒ skip)
// Forward decls (signatures match functions.h / existing impls — return type
// de SetAction en algunos headers es `void*`, así que delegamos vía el global
// header rather than re-declaring locally).
```

### Línea 2084 — antes de `void __cdecl FUN_0046c7f0(int param_1, int param_2, float param_3, float param_4, float pa`

```cpp
// ── FUN_0046c7f0 — movida desde stubs_game.cpp (refactor B3) ──
// FUN_0046c7f0 @ 0x0046C7F0 (~176 lines) — directional hit particles with blood
// AngleMatrix + VectorRotate para la dirección del impacto. Offset aleatorio por eje.
// param_1: 0=blood (red/green, type 0x4AB + AddTerrainLight), 1=hit spark (type 0x4C4), 2=hit spark variant
// param_2: puntero base de la entidad (posición en +0x10/+0x14/+0x18, ángulos en +0x1C, luz en +0xE8)
// param_3/4/5: direction angles for AngleMatrix
// FUN_0046c7f0 @ 0x0046C7F0 — Object_SpawnAmbientFX(kind, o, dx, dy, dz)
// Port FIEL de IDA `sub_46C7F0`. Es el spawner de fuego/humo de los objetos
// del mundo: lo llama `MoveObjects` (0x4FDC00) para los braseros, la forja del
// herrero, las chimeneas y los faroles.
//
//   kind 0 → llama de fuego: partícula 1195 (subtipo aleatorio 0..3, 50% de
//            las veces) + `AddTerrainLight` con tinte naranja (radio 4).
//   kind 1 → HUMO: partícula 1220, subtipo 0, 50% de las veces.
//   kind 2 → HUMO: partícula 1220, subtipo 2, 50% de las veces.
//
// (dx,dy,dz) es un desplazamiento LOCAL que se rota por los ángulos del objeto
// (`o+28`) — así el efecto sale del punto correcto del modelo (la boca de la
// chimenea, la punta del farol). Luego se le suma la posición del objeto
// (`o+16`) y un jitter de ±8 por eje.
//
// 2026-08-11: la versión anterior era una reinterpretación como "blood/spark"
// que (a) construía la matriz con (dx,dy,dz) COMO SI FUERAN ÁNGULOS y rotaba
// un offset (0,0,0) — o sea el desplazamiento se perdía y todo salía en el
// origen del objeto — y (b) usaba tipos de partícula 0x4AB/0x4C4 en lugar de
// 1195/1220. Resultado: ningún humo en el mundo.
```

### Línea 2144 — antes de `void CheckSprites(void)`

```cpp
// ── CheckSprites — movida desde stubs_misc2.cpp (refactor B3) ──
```

### Línea 2151 en `CheckSprites` — antes de `char *pcVar1 = DAT_07c85890;`

```cpp
    // Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba el dirty-mark.
```

### Línea 2159 — antes de `void __cdecl BuxConvert_0(int buf, int len) {`

```cpp
// ── BuxConvert_0 — movida desde stubs_misc2.cpp (refactor B3) ──
// ── Item data helper stubs ────────────────────────────────────────────────────
```

### Línea 2167 — antes de `int __cdecl Stats_CalcMagicDmgRange(int param_1) {`

```cpp
// ── Stats_CalcMagicDmgRange — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2248 — antes de `int __cdecl Stats_CalcAddStrength(short *param_1) {`

```cpp
// ── Stats_CalcAddStrength — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2358 — antes de `int __cdecl Stats_CalcDefense(int param_1) {`

```cpp
// ── Stats_CalcDefense — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2408 — antes de `int __cdecl Stats_CalcDefenseRate(int param_1) {`

```cpp
// ── Stats_CalcDefenseRate — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2482 — antes de `int __cdecl Stats_ExtraOptionEquip6(short *param_1) {`

```cpp
// ── Stats_ExtraOptionEquip6 — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2501 — antes de `int __cdecl Stats_ExtraOptionGlovesWings(int param_1) {`

```cpp
// ── Stats_ExtraOptionGlovesWings — movida desde stubs_bulk_small.cpp (refactor B3) ──
```

### Línea 2519 — antes de `int __cdecl Terrain_QuadEqual(int param_1, int param_2, int param_3, int param_4) {`

```cpp
// ── Terrain_QuadEqual — movida desde stubs_externs.cpp (refactor B3) ──
```

### Línea 2540 — antes de `void __cdecl VectorMA(float *va, float scale, float *vb, float *vc) {`

```cpp
// ── VectorMA — movida desde stubs_helpers.cpp (refactor B3) ──
// VectorMA @ 0x004F9CE0 — VectorMA(va, scale, vb, vc): vc = va + scale*vb
// IDA-ported: 3-vector multiply-add (Quake-style). Was stub copying in_rel.
```

### Línea 2549 — antes de `void __cdecl Vec3_Normalize(float *vec) {`

```cpp
// ── Vec3_Normalize — movida desde stubs_helpers.cpp (refactor B3) ──
```

### Línea 2557 — antes de `bool __cdecl Collision_SegmentToOBB(float *rayOrigin, float *rayTarget, const float *box)`

```cpp
// ── Collision_SegmentToOBB — movida desde stubs_mouse_hover.cpp (refactor B3) ──
// Collision_SegmentToOBB @ 0x00513260 — Entity_ViewportCheck(viewport, projection)
// Testea si la entidad descrita por 12 dwords (que el llamador copió de entity+0x130) está dentro
// del viewport actual, usando los punteros de matriz dados. Devuelve 1 si es visible, 0 si se descarta.
// Collision_SegmentToOBB @ 0x00513260 - test de interseccion SEGMENTO vs OBB por ejes
// separadores (SAT).  El "OBB" son los 12 floats que `Calc_RenderObject` deja en
// `objeto + 0x130` via `sub_4404E0`: centro (box[0..2]) y tres semi-ejes
// (box[3..5], box[6..8], box[9..11]).
//
// IDA prueba SEIS ejes y exige que TODOS solapen:
//     cross(dir, eje0), cross(dir, eje1), cross(dir, eje2), eje0, eje1, eje2
// con `dir = rayTarget - rayOrigin`.  La proyeccion de cada uno la hace
// `sub_5130F0` (FUN_005130f0), que ya estaba portada fiel mas arriba.
//
// 2026-09-04: antes era `return 1` con el comentario "STUB: frustum cull" -- o
// sea CUALQUIER objeto daba hit, y como el unico consumidor real
// (SpecialObject_HoverTest) estaba neutralizado, no se notaba.
```

## `src/Scene/Scene_Dispatch.cpp`

### Línea 74 en `Scene_Dispatch` — antes de `#if 0`

```cpp
    // ── RE-AUTH PACKET SEND (when keepalive counter > 0x1f) ──────────────────
    //
    // DISABLED for MuEmu compat (2026-04-25):
    //   Original 0.97 client sends a raw [C3][len][crc payload] re-auth packet
    //   every frame after Net_Connect via raw send() (no MuEmu cipher wrap).
    //   The packet body is XOR-encrypted with the login key, then CRC-wrapped.
    //
    //   Symptom: server FD_CLOSEs ~656 ms after F3/00 charlist arrives.  Login
    //   itself works because we wrap the F1/01 send through MuEmu::EncryptSend
    //   explicitly — but THIS path bypasses it, so once g_iNoMouseTime (re-auth
    //   keepalive counter) randomizes >0x1f, every frame leaks an unencrypted
    //   C3 onto the wire that MuEmu can't decode → server kills the socket.
    //
    //   Original target server expected this re-auth (it's a GameGuard ping).
    //   MuEmu doesn't, so we just skip the entire block.  The hash-table
    //   bookkeeping below still runs (anti-tamper ref-count, harmless).
```

## `src/Scene/Scene_EntityLifecycle.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

### Línea 8 — antes de `void __cdecl FUN_0050f700(const char* map_name)`

```cpp
// SaveMacro @ 0x0050F700 -- guarda Data\Macro.txt (10 lineas de hasta 256).
//
// 2026-09-24: estaba portada como "Map_Load" y ademas ROTA: abria con
// DAT_00559b74 ("rb") y llamaba `crt_fprintf(fp, &DAT_00560694)` diez veces
// sin pasarle el texto, o sea vaciaba el archivo de macros cada vez que
// corriera.  IDA (0x50F700) es sencilla:
//
//   v1 = fopen(FileName, "wt");
//   v2 = &unk_7E0FFC8;
//   do { fprintf(v1, "%s\n", v2); v2 += 256; } while ((int)v2 < (int)&ItemKey);
//   return fclose(v1);
//
// El bound `&ItemKey` (0x07E109C8) es el final del array: (0x7E109C8 -
// 0x7E0FFC8) / 256 = 10 entradas.  La llaman los tres puntos de salida de
// UI_InGameMenu, al lado de SaveOptionsToServer97k (sub_50F7A0).
//
// Guard propio del port: el original no chequea el fopen y deferencia el NULL.
```

### Línea 46 en `CreateObject` — antes de `int igx = (int)(param_2[0] * _DAT_00552d20);`

```cpp
    // BUG-FIX 2026-04-26 (audit #12): __ftol implementa truncate-toward-zero
    // (semántica de cast C de float→int), no nearest-even. lrintf redondeaba al
    // más cercano y divergía en negativos (lrintf(-1.5)=-2 vs __ftol(-1.5)=-1).
```

### Línea 126 en `CreateObject` — antes de `puVar3[3]=0x3f19999a; puVar3[0x19]=1;`

```cpp
            // IDA-faithful MODEL_MUGAME (case 3 en byte_4FFAA4[Type-60]):
            //   scale=0.6, [0x64]=1, [0xDC]=0. No escribe offsets 58/59/60
            //   (bodyLight). memset(0) previo deja bodyLight=0, pero lightEnable=0
            //   → nunca se lee. Revertido el "PORT FIX" previo porque el tint
            //   (1,1,1) no tenía efecto visual (lightEnable==0) y divergía de IDA.
```

### Línea 177 en `CreateObject` — antes de `puVar3[0xb]=puVar3[8]; puVar3[0xf]=puVar3[6]; puVar3[0xd]=puVar3[4];`

```cpp
            // Puertas de Devias (20/65/86/88).  Guarda el estado de REPOSO que
            // usa la animacion de apertura de `sub_4FDC00` case World 2:
            //   HeadAngle[0..2]       (+40,+44,+48) = Angle[0..2]
            //   HeadTargetAngle[0..2] (+52,+56,+60) = Position[0..2]
            // y normaliza el giro:  Angle[2] = HeadAngle[2] = (int)Angle[2] % 360.
            //
            // 2026-09-04 FIX: el `% 360` se hacia sobre los BITS del float, no
            // sobre su valor -- IDA es `(__int64)*((float *)v6 + 9) % 360`, o sea
            // truncar el float a entero y despues el modulo.  Leyendo los bits,
            // 90.0f (0x42B40000 = 1119092736) daba 336, 180.0f daba 224 y 270.0f
            // daba 112.  Consecuencia doble: la puerta quedaba girada un angulo
            // arbitrario (de ahi que se vieran mal puestas) y ademas los tests
            // `HeadAngle[2] == 90/270/0/180` del tick nunca matcheaban, asi que no
            // se abria.  Las unicas que funcionaban eran las de angulo 0, porque
            // los bits de 0.0f tambien son 0.
```

### Línea 207 en `CreateObject` — antes de `break;`

```cpp
        // 2026-09-04 FIX: aca habia un `[[fallthrough]]`.  IDA cierra el case 2
        // con `break` (0x4FF5A0 L292), y esa salida del switch exterior es la que
        // llega al `sub_4FF580` del final -- el LABEL_43 del decompile, o sea el
        // REGISTRO del objeto en la lista `Operates`.  Con el fallthrough los
        // tipos 22/25/40/45/55/73 de Devias caian en el `default: goto
        // lbl_skip_init` del case 3 y nunca se registraban, asi que no eran
        // clickeables (no hay silla donde sentarse).
```

### Línea 225 en `CreateObject` — antes de `break;`

```cpp
        // 2026-09-04 FIX: idem, IDA cierra el case 3 con `break` (L292).  Con el
        // fallthrough el tipo 8 de Noria (sentarse) no llegaba al registro.
```

## `src/Scene/Scene_LegacyCleanup.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 41 — antes de `// CSQuest__CheckQuestState @ 0x00401730`

```cpp
// 2026-09-25: los cinco puentes que habia aca (DeleteObjects, DeleteNpcs,
// DeleteMonsters, ClearItems y ClearCharacters) se eliminaron al renombrar:
// solo redirigian al FUN_ de la misma direccion, y con los dos lados ya con
// el mismo nombre quedaban llamandose a si mismos.  Las implementaciones
// reales viven en Render/SMD_Parser.cpp y Terrain/Terrain_LegacyLoad.cpp.
```

### Línea 48 — antes de `void __fastcall CSQuest_CheckQuestState(void *pThis, char param_1); // IDA: FUN_00401730`

```cpp
// CSQuest__CheckQuestState @ 0x00401730
// 2026-08-21: acá había un resumen inventado ("State machine dispatch
// (simplified)") que sólo escribía el byte de estado y descartaba el resto,
// mientras el port fiel de la misma dirección vive en Scene_CharSelect_Nav.cpp
// como CSQuest_CheckQuestState (IDA: FUN_00401730; despacha por estado 1/2/3 a CheckActCondition /
// FindQuestContext / CheckRequestCondition).  Ahora delega.
```

### Línea 60 — antes de `void __fastcall CSQuest_ShowDialogText(int param_1); // IDA: FUN_004017E0`

```cpp
// CSQuest__ShowDialogText @ 0x004017E0
// 2026-08-21: acá había una SEGUNDA implementación inventada (armaba el cuadro
// con una sola respuesta fija y no tocaba la tabla de diálogos), mientras el
// port fiel de la misma dirección vivía en Scene_CharSelect_Nav.cpp como
// CSQuest_ShowDialogText (IDA: FUN_004017E0). Dos implementaciones del mismo address escribiendo globals
// distintos — el patrón de siempre.  Ahora delega.
// El 2do parámetro no existe en IDA (`CSQuest::ShowDialogText(This, iDialogIndex)`
// es thiscall; el índice es el único dato que se usa).
```

## `src/Scene/Scene_LegacyHelpers.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.
```

### Línea 69 — antes de `HRESULT __cdecl Sound_ReleaseBuffer(int Buffer)`

```cpp
// Sound_ReleaseBuffer @ 0x00404AD0 (IDA: FUN_00404AD0; 5.2: ReleaseBuffer).
// Releases DirectSound buffers for the given slot (all loaded channels),
// resets slot count and 3D-anchor table.
//
// BUG-FIX 2026-04-28: el IDA original usaba `g_lpDSBuffer[0][v3]` con
// v3 = 4*Buffer + channel — un acceso flatten que el compilador C++ trata
// como "fila 0, índice fuera de rango". MSVC en Release lo computa offset-
// based (funciona) pero con ITERACIONES ilimitadas (MaxBufferChannel sin
// clamp) leía mucho más allá del array, devolviendo basura tipo 0xC2A00000
// (-80.0f bit-pattern) → v4->Release() → AV.
//
// Cambios:
//   1. Indexar con 2D plano: g_lpDSBuffer[Buffer][channel].
//   2. Clamp MaxBufferChannel a [0, 4] — array tiene exactamente 4 canales.
//   3. Bounds-check Buffer < 420.
//   4. Enable3DSound check usa el slot Buffer, no v2 (era bug del IDA).
```

## `src/Scene/Scene_Login_ServerSelect.cpp`

### Línea 132 en `Scene_Login_ServerSelect` — antes de `float       fVar4 = 0.0f;`

```cpp
    // fVar4: era "current Y of last drawn row" en Pass 3 (float-but-really-int).
    // Pass 4 la lee para posicionar la columna de canales. Inicializar a 0xdd
    // (221, el tope que asigna iStartY cuando hay poca lista) para que, si
    // ServerSelectHi != -1, Pass 4 use un Y razonable. El bug-fix en Pass 3 usa
    // iYNonPvp/iYPvp locales y sincroniza fVar4 al final.
```

### Línea 146 en `Scene_Login_ServerSelect` — antes de `float       uVar17;`

```cpp
    // BUG-FIX: era undefined4 (unsigned int). Al pasar a glColor3f(float,...)
    // hacia conversion int→float: 0x3f800000 → 1065353216.0f (se clampaba a 1.0).
    // Usar float directamente.
```

### Línea 167 en `Scene_Login_ServerSelect` — antes de `{`

```cpp
    // ── PASS 1: format channel name strings ──────────────────────────────────
    // IDA reference (0x0051F020):
    //   Single-ch : sprintf(buf, "%s %s", name, status)
    //   Multi NON-PVP channel: sprintf(buf, "%s-%d(Non-PVP) %s", name, chNum, status)
    //   Multi PVP channel:     sprintf(buf, "%s-%d %s", name, chNum, status)
    //   status = GlobalText[560/561/562]   (FULL/NORMAL/LOW)
    //
    // BUG-FIX vs Ghidra: las llamadas eran `crt_sprintf(buf, fmt)` sin args —
    // sprintf leía basura del stack y rendería "?TOO?TOO". Ahora pasamos los
    // args correctos. Hardcoded status strings (no tenemos GlobalText[] cargado).
```

### Línea 256 en `Scene_Login_ServerSelect` — antes de `int iYNonPvp;   // current non-PVP row Y (bumped +16 per entry)`

```cpp
    // Compute base Y for non-PVP column
    // BUG-FIX: Ghidra tipó los slots de Y como float, pero el asm original los
    // manipulaba como int32 (bit-pattern 0xdd=221). (int)3.08286e-43f→0, rompiendo
    // el cálculo. Reescrito con ints limpios: Y_top = min(0x1bc - 16*N, 0xdd).
```

### Línea 270 en `Scene_Login_ServerSelect` — antes de `fStack00000008 = (float)iYPvp;`

```cpp
        // Bridge: Pass 4/5 leen fStack00000008 como "PVP base Y" y fVar4 como
        // "non-PVP base Y" para posicionar la columna de canales.
        // BUG-FIX: fVar4 se quedaba en 0, haciendo que el panel de canales
        // non-PVP renderizara arriba de pantalla (Y negativo). IDA usa v10
        // (non-PVP base Y) en `v21 = 16 * v23 - 10 * v20 + v10 + 8`.
```

### Línea 355 en `Scene_Login_ServerSelect` — antes de `uVar6  = (uint)*((unsigned char*)&DAT_083a45ec + iVar8);`

```cpp
            // BUG-FIX: DAT_083a45ec es `*(DWORD*)(...)` lvalue → &DAT es DWORD*,
            // así que (&DAT)[iVar8] avanza iVar8*4 bytes. Disasm @ 0x0051f44e/4db
            // muestra `MOV AL, byte ptr [ECX + 0x83a45ec]` con ECX=iVar8 (byte
            // offset). Castear base a char* para byte arith.
```

### Línea 386 en `Scene_Login_ServerSelect`

```cpp
// BUG-FIX (ver arriba)
```

### Línea 405 en `Scene_Login_ServerSelect` — antes de `if ((*((unsigned char*)&DAT_083a4606 + (int)fStack00000018 + iVar8) & 0x80) == 0x80) {`

```cpp
                // Color by load
                // BUG-FIX: DAT_083a4606 es `*(WORD*)(...)` y DAT_083a4604 es
                // `*(DWORD*)(...)` lvalues. Indexar `(&DAT)[idx]` o `&DAT+idx`
                // multiplica el offset por 2/4. Disasm @ 0x0051f538/540:
                //   MOV AX,word ptr [ECX+EBP*1+0x83a4604]   ; channel_id
                //   MOV CL,byte ptr [ECX+EBP*1+0x83a4606]   ; load byte
                // ECX=server*0x21e, EBP=chan*0x1a (ambos byte offsets).
```

### Línea 448 en `Scene_Login_ServerSelect` — antes de `if ((*((unsigned char*)&DAT_083a4606 + ServerSelectHi * 0x21e + (int)fVar4) & 0x80) != 0x8`

```cpp
                // Load bar (only for non-full servers)
                // BUG-FIX (idem): WORD lvalue → necesita byte arith (char* cast)
```

### Línea 492 en `Scene_Login_ServerSelect`

```cpp
// BUG-FIX: byte arith
```

### Línea 498 en `Scene_Login_ServerSelect` — antes de `glColor3f(1.0f, 0.2f, 0.1f);   // orange-red (IP text)`

```cpp
        // BUG-FIX: literales 0x3fxxxxxx eran int → float value-cast. Usar literales float.
```

## `src/Scene/Scene_MapTick.cpp`

### Línea 27 en `Scene_MapTick` — antes de `GL_ResetState();`

```cpp
  // (was: int iStack0000000c — phantom outgoing-stack arg slot; resolved into RenderSkillTooltip 3rd param)
```

### Línea 32 en `Scene_MapTick` — antes de `if (DAT_07eaa134 != 0) {`

```cpp
  // IDA Scene_MapTick L32-38: RepairEnable_0 sólo se NORMALIZA a 1; nunca se
  // apaga acá.  El port tenía (2026-05-08) un "fix" que lo ponía en 0 cada
  // frame si RepairEnable (0x07EAA138) valía 0 -- y RepairEnable vale 0
  // siempre que la tienda está abierta (sub_4E6550), así que el modo reparación
  // del herrero duraba un frame y el click levantaba el item.
```

### Línea 125 en `Scene_MapTick` — antes de `if ((int)DAT_0055a3e4 >= 0 && (int)DAT_0055a3e4 < 20) {`

```cpp
  // 2026-05-05: clamp DAT_0055a3e4 to valid skill slot range (0..19) before
  // calling tooltip. Otherwise garbage values like 0x2A2A cause OOB reads
  // inside the tooltip code that crash on hover.
```

## `src/Scene/Scene_ObjectInteraction.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Scene_ObjectInteraction.cpp
//
// Extracted from stubs_game.cpp.  Owns special scene-object updates and
// mouse picking.  Entry-point comments retain their IDA symbols/addresses.
```

### Línea 11 — antes de `// PickObject_Mouse @ 0x004FA7C0 (~90 lines) — mouse-picking scene objects`

```cpp
//
// 2026-09-26: aca habia un puente con ese nombre cuyo cuerpo era
//     MoveObject_Special(param_1); return;   + 80 lineas despues del return
// o sea delegaba y dejaba la version vieja como codigo inalcanzable.  Nadie
// lo llamaba: los cuatro call sites van al FUN_ directo.  Eliminado.
```

### Línea 44 en `PickObject_Mouse` — antes de `__try {`

```cpp
            // 2026-05-07: SEH-wrap to survive corrupt next pointers in the
            // bucket linked list (same root cause as the MoveObjects guard).
```

## `src/Scene/Scene_ObjectLegacy.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 40

```cpp
// desactivado: el port FULL vive en stubs_IDA_ports.cpp
```

### Línea 41 — antes de `void __cdecl FUN_004fdc00(float pObj) {`

```cpp
// 0x004FDC00 — MoveObjects: tick per-frame de cada objeto visible del mundo.
//
// 2026-08-11 — UNIFICACIÓN. Existían DOS ports de esta función:
//   · éste, mínimo, que sólo hacía Alpha + el banner MUGAME del login, y
//   · `MoveObject_PerWorld` (stubs_game.cpp), con el toggle por HeroTile,
//     PlayAnimation y el switch por World COMPLETO.
// El que se llamaba desde el loop de MoveObjects era éste, así que el switch
// por World nunca corría: **ningún objeto del mundo generaba sus efectos**.
// Entre otras cosas, los tipos 130/131/132 de Lorencia (Light01/02/03) quedaban
// visibles como cajas de 8 vértices con la textura dummy `ston03` (2x2 negra)
// en vez de ocultarse (`HiddenMesh = -2`) y emitir el humo de las chimeneas y
// de la forja del herrero. Mismo patrón que `OpenSMDFile` / `RenderText` /
// `SetPlayerStop`: un símbolo con dos implementaciones donde gana la incompleta.
//
// Ahora hay una sola implementación, en el orden del binario:
//   World 9 → World 0/2 toggles → Alpha → early-return → PlayAnimation →
//   bloque de login (160/161/162) → switch por World.
```

## `src/Scene/Scene_ObjectTick.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

### Línea 13 — antes de `extern void __stdcall MoveObjects(void);`

```cpp
// Object_MoveUpdate — alias for MoveObjects (FUN_004FF260, per-frame
// world-objects animation/render-update dispatcher). The historical naming
// "Object_MoveUpdate" came from a Ghidra mis-id of FUN_0043E050 which is
// actually Movement_Tick (angle math). The CALLERS (Game_SceneUpdate /
// Game_EnterWorldTick / Game_CharSelectTick) want a per-frame objects tick,
// which IS MoveObjects (0x004FF260). Wire them here so the world-objects
// pool actually advances each frame.
//
// 2026-05-07 (revert): MoveObjects iterates the world-objects bucket
// grid (DAT_083a021c..) which is only properly populated when a world is
// loaded (state 5 = in-game). During Login/CharSelect/Loading the bucket
// linked-list pointers are uninitialized garbage → AV in FUN_004fdc00 →
// Alpha reading param_1 + 0x161. Gate on SceneFlag == 5 so this
// is only active in-game where the pool is real.
```

### Línea 34 en `Object_MoveUpdate` — antes de `if (SceneFlag == 2 || SceneFlag == 4) {`

```cpp
    // Login / CharSelect:
    // Keep this path inert for now. Multiple attempts to drive the login scene
    // objects from here ended in second-frame crashes, which strongly suggests
    // the original scene uses a narrower update path than the generic object
    // mover. We'll recover the logo / ship glows from the render side instead
    // of mutating login objects here.
```

## `src/Scene/Scene_ObjectUpdate.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Scene_ObjectUpdate.cpp
//
// Extracted from stubs_game.cpp.  Owns per-frame update/render dispatch for
// world scene objects and ambient bugs.  Function comments retain IDA provenance.
```

### Línea 11 — antes de `float* __cdecl MoveObject_PerWorld(float param_1) {`

```cpp
// MoveObject_PerWorld @ 0x004FDC00 (~608 lines) — SUMMARY STUB
// Per-world object animation. Per-frame for each visible scene object.
// World 9: random terrain lights. World 0: toggle objects by HeroTile.
// Then: Alpha(), BMD setup, animate, render via RenderPartObject.
```

### Línea 94 en `MoveObject_PerWorld` — antes de `if (SceneFlag == 2 || SceneFlag == 4) {`

```cpp
    // ── Escena de login / char-select (IDA sub_4FDC00, bloque previo al switch)
    // Este bloque vivía en la copia mínima de `FUN_004fdc00` (stubs_linker.cpp),
    // que era la que realmente se llamaba. Al unificar las dos copias se trae
    // acá, en el orden del binario: después de PlayAnimation, antes del switch.
    //   160 = Logo01 (cielo) y 161 = Logo02 (olas): scroll de la V de textura.
    //   162 = Logo03 (banner MU): rampa de Light + alpha-scalar. Sin esto el
    //         banner queda con bodyLight=(0,0,0) → rectángulo negro.
```

### Línea 443 en `MoveHeavenThunder` — antes de `int objectCount = 0;`

```cpp
    // Port fiel de IDA MoveHeavenThunder (0x004FED90).
    //
    // Antes era un ESQUELETO: calculaba la probabilidad y devolvia 1/0, pero las
    // dos llamadas que hacen el trabajo estaban solo como comentario
    // ("In original: complex phantom-register-based call"). La que faltaba y se
    // nota es `CreateEffect(182, ...)`: el tipo 182 es el modelo `cloud`
    // (`OpenWorldModels` case 10 hace `AccessModelWithTextures(182, "Data\Object11", "cloud", -1)`
    // + `OpenJPG("Effect\clouds.jpg", 1268)`), y `RenderEffects` lo dibuja por su
    // `case 182:`. O sea ESTE es el generador de las nubes de Icarus.
    //
    // Devuelve `objectCount` — un indice de objeto al azar que MoveObjects usa
    // para elegir a cual colgarle el rayo. El esqueleto devolvia 1 fijo.
```

### Línea 550 — antes de `void __stdcall MoveObjects(void) {`

```cpp
// MoveObjects @ 0x004FF260 (~169 lines) — per-frame object update dispatcher
// World 10: MoveHeavenThunder. World 11..16: ambient particles.
// Iterates all object lists calling MoveObject_Special or MoveObject_PerWorld.
```

### Línea 596 en `MoveObjects` — antes de `#define MOV_OBJ_VALID_PTR(p) \`

```cpp
            // 2026-05-07: guard against corrupt linked-list pointers.
            // The bucket grid's `next` field (+0x1B8) is sometimes garbage
            // (some unidentified path leaves a dangling pointer in a slot).
            // Wrap the deref in SEH so an AV reading the corrupt node terminates
            // the bucket walk instead of taking down the process. Range check +
            // iteration cap on top, to avoid loops that don't actually fault.
```

### Línea 626 en `MoveObjects` — antes de `{`

```cpp
                        // 2026-08-12 — BUG DE CONVERSIÓN, causa raíz de que
                        // NINGÚN objeto del mundo ejecutara su tick.
                        //
                        // `FUN_004fdc00` tiene la firma `(float o)` — un
                        // artefacto de Hex-Rays: el parámetro es un PUNTERO y
                        // adentro se usa siempre como `LODWORD(o)`, o sea por
                        // sus BITS. El call site hacía `(float)(DWORD)pcVar6`,
                        // que convierte el puntero NUMÉRICAMENTE: 0x12E37C8C
                        // (317752972) pasaba a 3.1775e8f, cuyos bits son
                        // 0x4D9749BE. `LODWORD(o)` recuperaba esa basura y la
                        // deferenciaba → AV que el `__except` de abajo se
                        // tragaba en silencio, abortando el walk del bucket.
                        // (El comentario "corrupt linked-list pointers" de ese
                        // SEH describía justamente ESTE puntero, no la lista.)
                        //
                        // Mismo primo del patrón `(float)(uintptr_t)` que
                        // corrompía los joints (ver CLAUDE.md 2026-08-10).
                        // El fix es reinterpretar los bits, no convertir.
```

### Línea 652 en `MoveObjects` — antes de `if (World == 10 && Scale != 0.0f) {`

```cpp
                        // 2026-09-03 -- RESTAURADO.  Otro agente removio este
                        // bloque concluyendo que "IDA 0x004FDC00 no tiene rama
                        // para World 10".  Eso es cierto para `sub_4FDC00` (el
                        // tick por objeto) pero el bloque NO vive ahi: vive en
                        // **MoveObjects (0x004FF260)**, la funcion que contiene
                        // este mismo loop, y ahi si esta, literal:
                        //
                        //   if ( World == 10 && objCount ) {
                        //     if ( rand() % 10 || (v5 = *(_WORD *)(v4 + 2), v5 < 0) || v5 > 5 )
                        //       --unk_83A3FEC;
                        //     else {
                        //       Light[i] = (double)(rand() % 10) * 0.02;
                        //       CreateSprite(1269, (float *)(v4 + 16), 0.5, Light, Hero, 0.0, 0);
                        //       Scale  = (double)(rand() % 20) + 10.0;
                        //       CreateJoint(1254, v4+16, v4+16, v4+28, 6, v4, Scale,  -1, 0);
                        //       Scalea = (double)(rand() % 20) + 10.0;
                        //       CreateJoint(1254, v4+16, v4+16, v4+28, 6, v4, Scalea, -1, 0);
                        //     }
                        //   }
                        //
                        // Son los rayos de tormenta que caen sobre los objetos
                        // del mapa en Icarus; el usuario confirmo en runtime que
                        // funcionaban ("ahi probe los truenos y aparecieron").
```

### Línea 713 en `MoveObjects` — antes de `if ((char*)puVar4 >= ((char*)&g_ObjectBucketGrid[0]) + 0x1000) {`

```cpp
        // BUG-FIX 2026-04-28: bound era abs addr 0x083a121b (= DAT_083a121c en
        // el binario original = end of g_ObjectBucketGrid[0x1000]). Cambiamos
        // a comparación contra el array end real.
```

### Línea 722 — antes de `void __stdcall MoveBugs(void) {`

```cpp
// MoveBugs @ 0x005001F0 — PORT FIEL 1:1 desde IDA (2026-07-16).
// Reemplaza el SUMMARY STUB previo (que omitía la rama LABEL_16 de las monturas
// e inventaba un "hover acotado" para el hada). Este es traducción directa de
// IDA sub_5001F0. Butterfly/mount/ambient update: chequea owner vivo, fade de
// alpha, copia pos del owner, dispatch de acción por CurrentAction del owner,
// avanza el BMD, y para el hada/uniria-helper (816/817) hace el follow-movement.
//
// Layout de entry (base = e, stride 0x1BC, 10 entries en DAT_083a1218):
//   +0x00 BYTE Live      +0x02 short type    +0x04 DWORD subType
//   +0x10 posX +0x14 posY +0x18 posZ   +0x1c angleX +0x20 angleY +0x24 angleZ
//   +0xC0 velX +0xC4 velY +0xC8 velZ   +0xCC animSpeed
//   +0xFC DWORD Owner    +0x105 curAction  +0x106 priorAction
//   +0x108 frame +0x10C priorFrame        +0x168 alpha-target
// Owner (CharactersClient) offsets: +0x10/14/18 pos, +0x1c/20/24 ang,
//   +0x7c(124) state, +0x84(132) Kind, +0x105(261) CurrentAction.
```

### Línea 884 en `MoveBugs` — antes de `Matrix_BuildFromEuler((float*)(e + 0x1c), (float*)(e + 0x90));   // AngleMatrix → scratch `

```cpp
            // PORT FIEL 1:1 (2026-07-18): el ASM (0x5007DB) escribe la matriz de
            // AngleMatrix en `[esi+0x90]` (campo scratch), NO en 0x24. Hex-Rays lo
            // decompiló como `(float*)v1+9`=0x24 pero es un artefacto: el disasm real
            // es `lea ebx,[esi+90h]`. angleZ (0x24) queda intacto → TurnAngle acumula
            // el giro y el hada ORBITA al char (movimiento tangencial cuando está lejos).
```

### Línea 920 — antes de `#define LODWORD(x)  (*(unsigned int*)&(x))`

```cpp
// === FUN_004fdc00 / MoveObjects (0x004FDC00) — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
// Estaba gateada por IDA_PORT_004FDC00, que esta definida: el gate era ruido.
// Es la copia VIVA: el `MoveObject_PerWorld` de mas arriba en este archivo es
// el resumen viejo y no tiene llamadores (su unico call site, en
// Scene_ObjectLegacy.cpp, esta bajo `#ifndef IDA_PORT_004FDC00`).
// Macros IDA locales para este port. #undef al final del bloque.
```

### Línea 929 en `Effect_PhysicsTick`

```cpp
// World-4 gate FX (stubs_game.cpp)
```

## `src/Scene/Scene_Objects.cpp`

### Línea 5 — antes de `#if 0`

```cpp
// Map_LoadObjectModels — port alternativo de 0x0050c4d0
// La versión activa (linkeada) vive en stubs.cpp:8626. Este port tiene strings
// distintos (ej. "SeaCreature" vs "SummonMonster") y quedó fuera del build.
// Se deja como documentación / referencia.
```

### Línea 149 en `Map_LoadObjectModels` — antes de `for (int texIdx = 0; texIdx < 32; ++texIdx) {`

```cpp
        // Load water tile textures into DAT_083a8ad8 array (32 entries × 0x38 stride).
        //
        // BUG-FIX 2026-05-03: bounds were absolute source-binary addresses
        // (`0x83a8d08`, `0x83a91d8`). DAT_083a8ad8 is now sized as `char[32*0x38]`
        // (was 1 byte → 32 strings of 0x38 bytes each = 1792 bytes of heap stomp
        // every time Icarus map loaded). Use indexed access; the first 10 entries
        // (offset 0..9 = bytes 0..560 = `< 0x230 from start`) get "wt0_%d.jpg"
        // (matching IDA literal 0x83a8d08 - 0x83a8ad8 = 0x230 = 10 × 0x38).
```

## `src/Scene/Scene_OpenWorld.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

### Línea 8 — antes de `void __cdecl OpenWorld(void) {`

```cpp
// IDA: OpenWorld (0x0050E5A0)
// Per IDA decompile (raw/0050E5A0_OpenWorld.c, 1500 bytes).
// Loads all terrain and tile textures for the current world map.
//
// World name = "World<N>" where N = World+1 (capped at 12 for dungeons 11-16).
// Loads Terrain.map, Terrain<N>.att, terrain.obj (or terrain<N>.obj for maps 2/3),
// TerrainHeight.bmp, TerrainLight.jpg, then 14 tile JPGs (slots 0x23-0x30) +
// 3 alpha-overlay TGAs (slots 0x32-0x34) + leaf01/02 + rain01/02 (always from
// World1) + rain03 (always from World10).
//
// BUG-FIX 2026-04-27: fixed path strings to match IDA exactly:
//   - "World_%d"           → "World%d"            (no underscore)
//   - "Data/%s/Terrain/%d" → "Data/%s/Terrain%d"  (no extra slash)
//   - "Data/%s/terrain/%d" → "Data/%s/terrain%d"  (no extra slash)
//   - rain01/02 use "World1" hardcoded; rain03 uses "World10" hardcoded.
//   - Pass FileName to OpenTerrainAttribute (was called with no args → no-op).
```

### Línea 35 en `OpenWorld` — antes de `memset(DAT_07abf5f0, 0, sizeof(DAT_07abf5f0));   // particle pool (3000×0x70)`

```cpp
    // BUG-FIX 2026-04-28: limpiar TODOS los pools de char-select que
    // sobreviven al world load. Sin esto los tick-functions iteran slots
    // con punteros garbage → AV.
```

### Línea 56 en `OpenWorld` — antes de `crt_sprintf(local_40, "Data/%s/EncTerrain%d.map", world_name, iVar2);`

```cpp
    // BUG-FIX 2026-05-01: los archivos reales en bin/Client/Data/World%d/
    // son EncTerrain%d.{map,att,obj} (versiones encrypted). El loader de
    // texturas tiene auto-fallback OZ*↔jpg/tga, pero los loaders de map/
    // att/obj NO. Sin esto los modelos cargan (Object*.bmd OK) pero las
    // INSTANCIAS (qué objeto va dónde) jamás se leen → mapa renderiza
    // solo terreno + hero, sin casas/NPCs estáticos/props.
```

### Línea 65 en `OpenWorld` — antes de `crt_sprintf(local_40, "Data/%s/Terrain%d.att", world_name, iVar2);`

```cpp
    // 2026-05-04: el archivo `EncTerrain%d.att` mide 131076 bytes (formato
    // encriptado custom) pero `OpenTerrainAttribute` solo acepta 65539 bytes
    // (formato vanilla 0.97k). Sin .att cargado → DAT_0838bc70 queda en 0
    // → todas las tiles son walkable → atravesamos casas y NPCs.
    // Intentamos el archivo unencrypted `Terrain%d.att` primero (mismo formato
    // que IDA espera). Fallback a EncTerrain*.att si no existe.
```

### Línea 83 en `OpenWorld` — antes de `crt_sprintf(local_40, "%s/TerrainHeight.OZB", world_name); CreateTerrain(local_40);`

```cpp
    // BUG-FIX 2026-05-01: archivos reales en filesystem son OZ* (encrypted),
    // no .bmp/.jpg/.tga. La función OpenJPG no hace ext-swap automático
    // a menos que DAT_0055a7c4 != 0 — y en in-game está en 0. Usamos extensiones
    // reales directamente para que fopen abra el archivo correcto.
```

## `src/Scene/Scene_ServerSelect_Input.cpp`

### Línea 60 en `CServerSelWin_UpdateWhileActive` — antes de `for (int iVar12 = 0; iVar12 < 0x34ee; iVar12 += 0x21e) {`

```cpp
    // IDA: for ( i = 0; i < 13550; i += 542 )  — 25 iterations (0x34ee / 0x21e).
    // BUG-FIX: `(&DAT_083a45ec)[i]` es DWORD* (scale 4). IDA usa byte arith:
    //   `*((_BYTE*)&unk_83A45EC + i)`. Sin el cast leíamos memoria equivocada.
```

### Línea 85 en `CServerSelWin_UpdateWhileActive` — antes de `const int iYNonPvpBase = iVar12;`

```cpp
    // BUG-FIX: guardar Y base ANTES de que Pass 2 mute iVar12/iVar4.
    // IDA: v38 = v3 (non-PVP base), v40 = v7 (PVP base) — usadas en Pass 3.
    // Antes Pass 3 leía iVar12/iVar4 post-Pass-2 → hit-area 16px debajo del render.
```

### Línea 175 en `CServerSelWin_UpdateWhileActive` — antes de `} while ((int)local_830 < (int)((const char*)&DAT_083a45ed + 0x34ee));`

```cpp
    // BUG-FIX: bound absoluto 0x83a7adb era la dirección ORIGINAL del binario
    // (0x083a45ed + 0x34ee). En nuestro build DAT_083a45d8 está en otra
    // dirección asignada por el linker → el loop nunca terminaba y walking
    // memoria inválida. IDA: `while ((int)v6 < 138050267)` = mismo bug.
    // Fix: bound relativo al base del array.
```

### Línea 204 en `CServerSelWin_UpdateWhileActive`

```cpp
// BUG-FIX: reuse byte-correct numCh from above
```

### Línea 220 en `CServerSelWin_UpdateWhileActive` — antes de `ServerLocalSelect = (unsigned int)`

```cpp
                // BUG-FIX byte-arith: DAT_083a4604 es *(DWORD*) y DAT_083a4606
                // *(WORD*) lvalues → `&DAT + i` / `(&DAT)[i]` escalan el offset
                // ×4/×2. Con el server en slot 23 (CS) eso leía FUERA del array.
                // Igual que en el render (Scene_Login_ServerSelect), castear a
                // char*/unsigned char* para aritmética de bytes.
```
