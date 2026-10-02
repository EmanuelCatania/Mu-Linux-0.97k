# Historial de comentarios: `carpetas chicas de src/`

Comentarios de desarrollo movidos desde `carpetas chicas de src/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Debug/MiniDump.h`

### Línea 31 — antes de `static void Preload();`

```cpp
    // Escribe el .dmp en el directorio de trabajo.  Devuelve true si lo logro y
    // deja la ruta en outPath.  Es seguro llamarla desde dentro de un filtro de
    // excepciones: no aloca ni usa la CRT mas alla de wsprintf.
    // Carga dbghelp.dll y resuelve MiniDumpWriteDump POR ADELANTADO, y aparta
    // una reserva de memoria de emergencia.  Hay que llamarla al arrancar,
    // junto a SetUnhandledExceptionFilter.
    //
    // No es una optimizacion: si el crash es por falta de memoria -- que es
    // justo el caso del reporte del 2026-09-30, un std::bad_alloc tras una hora
    // de leak -- LoadLibrary no puede mapear un modulo nuevo y el dump no se
    // escribe nunca.  Se perdia el .dmp precisamente en el crash donde mas
    // falta hacia.
```

## `src/Entity/Entity_ActionLegacy.cpp`

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

### Línea 48 — antes de `void* __cdecl SetAction(int entity_ptr, int anim_id)`

```cpp
// BUG previo: agregábamos un deref *(DWORD*)slot que leía los primeros 4 bytes
// del modelName ("Play"=0x79616C50) como puntero y crasheaba en *(short*)(0x79616C50+38).
```

## `src/Entity/Entity_BoneEffects.cpp`

### Línea 1 — antes de `// stubs_helpers.cpp`

```cpp
// Entity_BoneEffects.cpp
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

### Línea 53 en `Entity_SpawnBoneEffect` — antes de `float offset[3];`

```cpp
    // offset vector at bone position + x/yOff
    // BUG-FIX 2026-08-18 (A): el vector de offset se pasaba desde `&offset[3]`,
    // o sea leia offset[3],[4],[5] - dos floats FUERA del array. IDA sub_456590
    // arma v9[0]=a5, v9[1]=a6, v9[2]=a7 y pasa v9, el indice 0.
```

### Línea 74 en `Entity_SpawnBoneEffect` — antes de `CreateSprite((unsigned short)effectType, outPos, scale, light, entity, 0.0f, 0);`

```cpp
    // BUG-FIX 2026-08-18 (B): el 3er argumento de CreateSprite es la ESCALA y se
    // pasaba (float)effectType - o sea escala 1191 para el tipo 1191. IDA
    // sub_456590: CreateSprite(Type, Position, Scale, Light, Owner, 0.0, 0).
    // Con eso el quad media 152448 unidades y, pintado con Light=(v, v*0.6,
    // v*0.4) = salmon, tapaba la pantalla entera en Atlans.
```

## `src/Entity/Entity_HeadMotion.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 88 en `MoveHead` — antes de `LookAtTarget((DWORD)param_1,`

```cpp
        // Combat: mirar al blanco del ataque.
        //
        // 2026-09-04 FIX (cabezas de los monstruos girando como locas / sin
        // cabeza): faltaba el STRIDE.  IDA (0x43E940) hace
        //     sub_43E890(a1, COERCE_FLOAT(CharactersClient + 916 * v7))
        // con `v7 = *(_WORD *)(a1 + 784)` = el indice de la entidad objetivo.
        // Aca se pasaba `CharactersClient + indice`, o sea un puntero al medio
        // del struct de la entidad 0: LookAtTarget leia su "posicion" de campos
        // arbitrarios y el angulo de cabeza salia disparado a cualquier lado.
        // 916 = 0x394 es el stride del array de entidades.
```

## `src/Entity/Entity_LegacyEffects.cpp`

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

## `src/Entity/Entity_LegacyPoolCleanup.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"
```

### Línea 93 en `DeleteCloth` — antes de `DWORD* end = (DWORD*)((char*)&DAT_07b11670[0] + sizeof(DAT_07b11670) + 4);`

```cpp
    // 2026-09-04: el bound era 124 slots.  El pool de efectos tiene 200
    // (IDA acota con `&unk_7B27154`: (0x07B27154 - 0x07B11674) / 0x1BC = 200) y
    // ya se habia redimensionado en 2026-08-15; esta funcion quedo con el valor
    // viejo, asi que los efectos de los slots 124..199 no se borraban nunca.
```

### Línea 109

```cpp
// 2026-09-25: aca habia un puente FUN_00460d20 sin callers que solo llamaba a
// DeleteEffect (misma direccion, 0x00460D20).  Eliminado.
```

## `src/Entity/Entity_LegacyTeleport.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"
```

### Línea 47 — antes de `extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType)`

```cpp
// (61 bytes) — walks Joints pool DAT_07b27150 (500 × 0x9D8) and zeroes any
// active slot whose Type/Target/SubType match.
//
// Slot layout per IDA `CreateJoint` (raw/0046D840_CreateJoint.c:178-195):
//   slot+0   byte  active flag
//   slot+4   int   Type
//   slot+8   int   SubType
//   slot+64  DWORD Target  (= entity ptr / owner)
// Stride 0x9D8 = 2520 bytes per slot.
//
// 2026-05-08: previously a no-op (mc_DeleteJoint in SecondPassword.cpp) "until
// joint pool wired". Pool IS sized in globals.cpp (200×0x9D8); now functional.
```

### Línea 74 — antes de `// IDA: FUN_004792C0 (0x004792C0)`

```cpp
// 2026-09-25: aca habia un puente FUN_0046fe00 sin callers que solo llamaba a
// DeleteJoint (misma direccion, 0x0046FE00).  Eliminado.
```

### Línea 77 — antes de `extern void __cdecl Entity_TeleportAnim(float* world_pos,`

```cpp
// IDA: FUN_004792C0 (0x004792C0)
// CreatePoint(float Position[3], int Value,
//   float Color[3], float scale)  (101 bytes)
//
// Spawns a damage popup / floating text in the point pool DAT_07c80110
// (100 × 0x70 bytes). Per IDA decompile (raw/004792C0_CreatePoint.c).
//
// 2026-05-08: this address is the same function as the existing
// `Entity_TeleportAnim` in this file — the alias was misnamed because
// `Skills.cpp:349` calls it expecting a teleport-spawner, but the actual IDA
// is `CreatePoint` (damage-popup spawner). The Net_Process case 0x15 path
// uses it correctly as damage popup. We add the IDA name as the canonical
// entry point and the existing `Entity_TeleportAnim` impl provides the body.
//
// Skill teleport effect is actually `CreateTeleportBegin` (FUN_004742b0) /
// `CreateTeleportEnd` (FUN_00474310) — Skills.cpp:349 is a misrouted call
// (left as-is for now; spawning a degenerate damage popup is harmless, and
// fixing the routing belongs to Skills.cpp).
//
// `Entity_TeleportAnim` body (lower in this file, ~line 1070) is the actual
// implementation; this wrapper just exposes the IDA name.
```

### Línea 109 — antes de `extern "C" void __cdecl CreateTeleportBegin(unsigned int o)`

```cpp
// FUN_004742b0 @ 0x004742B0 — CreateTeleportBegin(DWORD o)  (83 bytes)
// Per IDA decompile (raw/004742B0_CreateTeleportBegin.c). Begins teleport
// animation: anim 87, alpha=0 (fade out), state byte 1, sparkle effect 1176.
// Sound 88 (whoosh).
//
// 2026-05-08: previously aliased as `Entity_WeaponHit` in CLAUDE.md and our
// Combat.cpp comments, but IDA confirms this is the teleport-begin function.
// SetAttackSpeed / SetAction / CreateEffect / PlayBuffer decls in functions.h.
```

### Línea 130 — antes de `extern "C" void __cdecl CreateTeleportEnd(unsigned int o)`

```cpp
// FUN_00474310 @ 0x00474310 — CreateTeleportEnd(DWORD o)  (93 bytes)
// Per IDA decompile (raw/00474310_CreateTeleportEnd.c). Completes teleport:
// anim 87, anim_speed +0x108=4.5f (0x40A00000), state byte 3, alpha=1.0f
// (fade-in), sparkle effect 1176, sound 88.
```

## `src/Entity/Entity_Lookup.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

### Línea 11 — antes de `int __cdecl Entity_FindById(int entity_id) {`

```cpp
// 2026-05-07: fixed to match IDA — previously returned 0 on miss + ignored the
// active flag, which caused PacketHandler_0x5c writes for unknown entities to
// land on (potentially NULL or player) slot 0.
```

## `src/Entity/Entity_MoveClient.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

## `src/Entity/Entity_Spawn.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// IDA: CreateCharacterPointer (0x0045ADC0)
// Initialize/spawn an entity.
//
// Ghidra-confirmed signature:
//   void __cdecl CreateCharacterPointer(CHARACTER* c, int Type,
//                                       int PositionX, int PositionY,
//                                       float Rotation)
//   c         = entity pointer (stride 0x394 from DAT_07abf5d0)
//   Type      = object type code → entity[+0x02] (e.g. 0x186 for hero placeholder)
//   PositionX = grid X coordinate → entity[+0x388]
//   PositionY = grid Y coordinate → entity[+0x38c]
//   Rotation  = initial facing angle (stored into Object.Angle)
//
// ── Hash-table obfuscation note ───────────────────────────────────────────────
// Lines 1-480 (of 797) of the Ghidra decompile consist almost entirely of
// HashTable encode/decode operations on (param_1+0x388) and (param_1+0x38c)
// — the two grid coordinate slots. The anti-tamper XOR-encodes those words,
// then decodes them immediately to use in the terrain-attribute test below.
// Per CLAUDE.md policy these are reference-count obfuscation, not game logic.
// The effective result of those 480 lines is:
//   param_1[0x388] = grid_x  (param_2 & 0xff)
//   param_1[0x38c] = grid_y  ((param_2 >> 8) & 0xff)
// — exactly what we write directly here.
//
// Lines 481-797 contain the real initialization logic, fully implemented below.
//
// ── Entity struct offsets used ────────────────────────────────────────────────
// +0x00  = active flag (byte, 1=active)
// +0x02  = entity_type (short)
// +0x04  = unk_04 (int)
// +0x0c  = scale base (float, init 0.9)
// +0x10  = world_x (float)
// +0x14  = world_y (float)
// +0x18  = world_z (float)
// +0x1bc = move_type_flags (byte)
// +0x1c  = unk_1c
// +0x1da = unk_1da (short, init 0xffff)
// +0x1e2 = equip slot 0..5 (stride 0x18, 6 slots)
// +0x272 = equip slot 6..7 (stride 0x18, 2 slots)
// +0x2a0 = unk_2a0 (short, init 0xffff)
// +0x2b8 = char_class (short, init 0xffff)
// +0x2d0 = unk_2d0 (short, init 0xffff)
// +0x2e9 = unk_2e9 (byte)
// +0x2ea = unk_2ea (byte, init 3)
// +0x2eb = tipo de monstruo (byte).  CreateCharacterPointer lo deja en 0xFF
//          (= sin tipo, el valor del heroe) y CreateMonster escribe el Type real.
// +0x2ec = unk_2ec (byte)
// +0x2ed = unk_2ed (implicit)
// +0x2fa = unk_2fa (short, init 10)
// +0x2fd = render_visible (byte)
// +0x2fe = unk_2fe (byte)
// +0x300 = stamina_counter (byte)
// +0x305 = unk_305 (byte)
// +0x306 = target_grid_x (byte)
// +0x307 = target_grid_y (byte)
// +0x330 = unk_330 (int, init 0xffffffff)
// +0x334 = unk_334 (int)
// +0x33c = unk_33c (int, init 0xffffffff)
// +0x340 = g_framecount mirror (int)
// +0x344 = unk_344
// +0x34e = SafeZone (byte) — TerrainWall[Terrain_Load(x,y)] & 1. NO es dead
//         (el dead real es +0x2FD, IDA ReceiveDie L18).
// +0x34f = unk_34f (byte)
// +0x354 = path_current_wp (byte)
// +0x356 = path_wp_count (byte)
// +0x388 = cached_wp_x (int)
// +0x38c = cached_wp_y (int)
// +0x17c = base_level (int)
```

### Línea 274 en `CreateCharacterPointer` — antes de `if (*(unsigned char **)(param_1 + 0x114) != NULL) {`

```cpp
    // ── BoneTransform2 buffer allocation ──────────────────────────────────────
    //
    // Field entity[+0x114] es BoneTransform2: array de matrices 3×4 (48 B=0x30)
    // por bone.  BMD_Animation (Sprite_Draw) itera hasta model[+0x22] (boneCount)
    // y escribe cada bone en buf[boneIdx*0x30]. El buffer necesita al menos
    // `boneCount * 0x30` bytes.
    //
    // ASM original (0045bb70..0045bb8e):
    //   MOV EAX,[0x05828d58]                       ; Models base ptr
    //   MOVSX EAX, word ptr [EAX + type*0xbc + 0x22] ; bone count
    //   LEA ECX,[EAX + EAX*2]                      ; count*3
    //   SHL ECX,0x4                                ; count*0x30
    //   PUSH ECX; CALL operator_new
    //
    // BUG-FIX: el port usaba offset +0x1022 (de una mala interpretación del
    // decompile Ghidra que mostraba "Models[0xee8ef].Data + 0x9e"), lo que leía
    // 22 structs MODEL_t adelante y devolvía un short basura. Si ese short era
    // menor que el bone count real → undersized buffer → overflow detectado por
    // PageHeap en R_ConcatTransforms línea 7266 (crash al escribir el último bone).
    // Correcto: offset +0x22 dentro del struct MODEL_t (mismo que usa
    // BMD_Animation para su loop count).
```

### Línea 304 en `CreateCharacterPointer` — antes de `if (boneBuf && boneCount > 0)`

```cpp
        // UB heredado del original: IDA 0045ADC0 L710 hace `operator_new` sin
        // inicializar, y el buffer se LEE antes de escribirse. El tick del frame
        // del spawn (MoveCharacterClient -> MoveCharacterVisual, 0x4520C0) entra
        // al switch por ModelID y transforma huesos que Calc_RenderObject todavia
        // no lleno: recien los llena el render, que corre despues.
        //
        // En el binario release eso devuelve paginas frescas del OS (ceros) y el
        // artefacto no se ve. Con el CRT debug el relleno es 0xCDCDCDCD, o sea
        // posiciones de ~-5.6e8: la cadena de 13 joints 1254 de Queen Rainer
        // (ModelID 321) nace con esas coordenadas y dibuja los haces azules que
        // cruzan la pantalla. Cuadra con el sintoma: al entrar por primera vez
        // los mobs salen mal y al alejarse y volver (slot ya con huesos validos)
        // se ven bien.
        //
        // Se inicializa a cero, que es lo que el original obtiene de hecho.
        // Mismo criterio que el fix de los buffers POT de textura (7c1a39d).
```

## `src/GameGuard/GameGuard_InitTrampoline.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comment retained.
```

## `src/GameGuard/GameGuard_LegacyHealth.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 41 — antes de `void __cdecl FUN_0053d430(unsigned char *gameName) {`

```cpp
// Ver CLAUDE.md, seccion GameGuard, para la cadena completa y que haria falta.
```

## `src/GameGuard/GameGuard_Packet.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_bulk_misc.cpp; IDA provenance comments retained.
```

## `src/Input/Cursor_Render.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

### Línea 8 — antes de `void __cdecl Cursor_Render(void) {`

```cpp
// IDA: RenderCursor @ 0x004BFFA0 — Cursor_Render.
// Draws the in-game mouse cursor sprite. Sprite ID selected by:
//   game_substate, hovered entity type, cursor-mode flags (DAT_00559C48/4C/50/54).
// Uses FUN_005125A0(sprite_id, x, y, 24, 24, u, v, 1, 1) for fixed sprites,
// or GL_DrawRotatedRect for animated/colored variants.
// Cursor offset = _DAT_0055264C from mouse pos (DAT_083A427C/78).
// Reescrito 1:1 con IDA `RenderCursor` (004BFFA0_RenderCursor.c, 152 líneas).
// Decisión de sprite por prioridad:
//   SelectedItem       → bitmap 5   (item ground)
//   SelectedNpc        → bitmap 6   (NPC, animación 3×2 uv via u/v)
//   SelectedOperate    → bitmap 8 (world match) / 9 (genérico)
//   !Hero.dead && SelectedCharacter:
//       CheckAttack && !MouseOnWindow → bitmap 4 (attack target)
//       else                          → bitmap 2 (arrow, LABEL_43)
//   RepairEnable == 1  → bitmap 7
//   RepairEnable == 2  → bitmap 7 (animado, sin(WorldTime*0.02))
//   !MouseLButton      → bitmap 2 (arrow, LABEL_43)
//   MouseLButton && DontMove  → bitmap 10
//   MouseLButton && !DontMove → bitmap 3 (move)
//
// NULL-guard sobre Hero (DAT_07abf5d8): en el original el crash acá era
// imposible porque SelectedCharacter=-1 en login y Hero siempre apuntaba a
// una entidad válida in-game; acá Hero=NULL en login si aún no se asignó.
// IDA: RenderCursor
```

### Línea 37 en `Cursor_Render` — antes de `int frame = (int)(long long)((double)DAT_05826e08 * 0.0099999998) % 6;`

```cpp
    // Frame = (int64)(WorldTime * 0.01) % 6  — IDA lo emite con __int64 cast explícito.
    // 2026-09-03 -- ANIMACION DEL CURSOR CONGELADA (y de todo lo que depende
    // de WorldTime).  IDA: `Frame = (int)(__int64)(WorldTime * 0.0099999998) % 6;`
    // -- multiplica el FLOAT y recien despues convierte a __int64.  El port
    // castea a `int` ANTES, y WorldTime = timeGetTime() pasa de 2^31 ms a las
    // ~24.8 dias de uptime de la maquina: el cast satura y el frame queda
    // clavado.  Por eso el cursor sobre NPC se ve estatico en una maquina con
    // mucho uptime y normal en una recien reiniciada.  Habia 9 sitios iguales.
```

## `src/Input/Input.cpp`

### Línea 262 — antes de `int __cdecl PressKey(int param_1)`

```cpp
// IDA: PressKey (0x0047EC20)
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
// IDA: PressKey
```

## `src/Input/Input_Timing.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

## `src/Input/Mouse_Hover.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Mouse_Hover.cpp — IDA: FUN_004b0310 — Mouse_UpdateHoverTargets
// Per-frame mouse cursor billboard render + hover target determination.
//
// Called every frame. Two responsibilities:
//   1. Render the cursor as a 2D billboard sprite.
//   2. Determine which entity/item is under the cursor and store hover targets.
//
// ── Cursor billboard ──────────────────────────────────────────────────────────
// RenderTerrainTile(DAT_080ab288, DAT_080ab28c, screenX, screenY, 1.0f, 1, 1):
//   Returns nonzero if cursor is visible/active.
// If visible, calls GL_SetBlendAdditive() (hide char anim sprite for cursor area),
// then RenderTerrainAlphaBitmap(type=8, x, y, sx, sy, color, 0, alpha) to draw the quad.
//   - States 2/4/5 (login/charselect/ingame): fixed size based on DAT_07e11d5c
//   - States 1/3 (intro/loading): animated size using DAT_07e11d5c oscillation
//
// ── Hover targets ─────────────────────────────────────────────────────────────
// After cursor render, sets hover indices:
//   SelectedItem = item on ground (-1=none)
//   SelectedNpc = NPC / shop entity (-1=none)
//   SelectedCharacter = mob or player entity (-1=none)
//   SelectedOperate = special object (-1=none)
//   Attacking = secondary hover (cleared if SelectedCharacter resets)
//
// Priority with Alt held (VK_MENU):
//   item-ground (ItemOnGround_HoverTest, IDA: FUN_004afa40) → NPC type 4 → mob type 0x22 → player type 1 → special
//
// Priority without Alt + swim/idle:
//   mob/player type 0x22 → type 1 → NPC type 4 → item-ground → special
//
// HashTable section (lines ~131-350 in original):
//   Anti-tamper XOR encode/decode block operating on DAT_07cf1ffc (0x584-byte
//   char-data buffer). Per CLAUDE.md policy, hash table operations are
//   reference-count obfuscation — not game logic. Omitted from implementation.
//
// After hover detection: if hover target found and Attacking != -1,
//   calls FUN_004afb00() to process the pending click action.
```

### Línea 50 en `Mouse_UpdateHoverTargets` — antes de `#if 0`

```cpp
    // ── 1. Cursor billboard render ────────────────────────────────────────────
    // 2026-04-29 DISABLED: el cursor billboard 3D (sprite en el suelo del tile
    // hovered) requiere DAT_07eab24c (BackTerrainHeight) que no se inicializa
    // en nuestro port. Crash AV en Terrain_RenderQuad al acceder al buffer null.
    // El cursor 2D (Cursor_Render) sigue funcionando normalmente.
```

### Línea 82 en `Mouse_UpdateHoverTargets` — antes de `if (m_bAutoAttack == '\0' || World == 6) {`

```cpp
    // 2026-05-06: añadido guard `c50 >= 0` para evitar OOB read cuando
    // SelectedCharacter == -1 (initial state). Antes se leía entity[+0x2fd] con
    // c50=-1 → puntero negativo → crash latente.
```

### Línea 92 en `Mouse_UpdateHoverTargets` — antes de `if (Attacking == -1 ||`

```cpp
        // Current hover target is a valid alive monster.
        // 2026-05-06: REMOVED reset on IsClickPushed/DAT_083a42c4. La lógica
        // original IDA reseteaba aquí porque la detect que sigue inmediato
        // re-poblaría. Pero en nuestro port el detect a veces falla (terrain
        // filter, etc) → c50 quedaba -1 al click time → mob attack handler
        // no disparaba. Mantener el target HASTA que detect lo reemplace.
        // Keep the original transient hover state.  Leaving this selected
        // after the cursor moves away turns later ground clicks into a basic
        // attack against the stale mob.
        // IDA sub_4B0310 L85-106 — el bloque de reset completo:
        //   if ( !m_bAutoAttack || World == 6 )      { SelectedCharacter = -1; Attacking = -1; }
        //   else if ( !target->Dead && target->Kind == 2 )
        //   {
        //       if ( Attacking == -1 || MouseLButton || MouseLButtonPush
        //         || MouseRButton || MouseRButtonPush || Hero->Dead )
        //           SelectedCharacter = -1;
        //   }
        //   else { Attacking = -1; SelectedCharacter = -1; }
        //
        // El clear NO es destructivo: el detect que viene justo despues
        // (`if (SelectedCharacter == -1) SelectedCharacter = sub_4AFDC0(...)`,
        // L326) lo vuelve a poblar con lo que haya bajo el cursor.  Por eso el
        // objetivo sigue al mouse en el original.
        //
        // El port tenia SOLO los dos flags del boton DERECHO
        // (DAT_083a42ac / MouseRButtonPush), asi que clickeando con el IZQUIERDO el
        // target nunca se limpiaba: quedaba pegado el primer mob que hubiera
        // pasado por debajo del cursor.  Direcciones confirmadas con
        // ida_xrefs_to:  MouseLButton = 0x083A42C4 · MouseLButtonPush = 0x083A4124
        //                MouseRButton = 0x083A42AC · MouseRButtonPush = 0x083A42D0
        //                m_bAutoAttack = 0x00559C5C · Attacking = 0x00559C58
```

### Línea 182 en `Mouse_UpdateHoverTargets` — antes de `int firstKind = 0x22, secondKind = 1;`

```cpp
            // IDA sub_4B0310 L315-351 (Alt SIN apretar): cadena de descarte
            // estricta, personaje -> personaje -> NPC -> ITEM -> mobiliario.  El
            // item solo se elige si el cursor no esta sobre ningun personaje ni NPC.
            //
            // 2026-09-21: aca habia una inversion puesta el 2026-07-27 que miraba
            // el item PRIMERO, porque "cualquier mob cercano en pantalla robaba el
            // hover".  Esa causa desaparecio el 2026-09-16 (2f83d26): desde ahi
            // Entity_SelectNearest usa el rayo contra la OBB, como IDA, y solo
            // elige al que esta realmente bajo el cursor.  La inversion quedo
            // compensando un problema que ya no existia, y su efecto era el
            // reporte del tester: con un item debajo del monstruo el cursor
            // quedaba en el de levantar en vez del de ataque (RenderCursor le da
            // prioridad a SelectedItem).  Con Alt APRETADO los items si van
            // primero -- esa rama de arriba es la de IDA y no se toca.
            //
            // Orden de los dos tipos de personaje (IDA L117-118 y L318-322): por
            // defecto monstruos (0x22) y despues jugadores (1); con un buff de
            // elfa activo (skills 26-28: curar, mas defensa, mas dano) se invierte,
            // para poder apuntarle a un jugador que tiene un monstruo detras.
```

### Línea 242 — antes de `int __cdecl RenderTerrainTile(int iparam_1, int iparam_2, int param_3, int param_4, float `

```cpp
// ── Additional helpers extracted from stubs_mouse_hover.cpp ─────────────────
```

### Línea 289 en `RenderTerrainTile` — antes de `for (int i = 0; i < 4; ++i) {`

```cpp
        // BUG-FIX 2026-04-28: bound era 0x7feb288 (addr abs del binario original).
        // Pool real es g_TilePickBuf[12] = 4 vec3 corners. Iterar 4.
```

### Línea 328 en `RenderTerrainTile` — antes de `for (int i = 0; i < 4; ++i) {`

```cpp
            // BUG-FIX 2026-05-03: was `while (puVar3 < 0x7feb288)` — absolute
            // source-binary bound, junk in our build. g_TilePickBuf[12] holds
            // exactly 4 vec3 corners (matching the lines 924 fix above).
```

### Línea 384 en `Entity_SelectNearest` — antes de `float best_perp = 1e12f;`

```cpp
    // Pass 2: find nearest entity to MOUSE-RAY (perpendicular distance), not camera.
    // Antes: usábamos distancia a cámara con Collision_SegmentToOBB stub → siempre return 1
    // → ganaba el más cercano a cámara siempre, que es slot 1 (elfa) por geometría.
    // Ahora: gana el char cuyo centro de masa está más cerca del ray del mouse.
```

### Línea 431 en `Entity_SelectNearest` — antes de `{`

```cpp
        // 2026-09-16: aca habia una reimplementacion en pantalla (gluProject de
        // tres puntos a 10/40/70 de altura sobre los pies y un radio fijo de 32
        // px).  Apuntando a la parte alta del cuerpo, o con el mob inclinado en
        // su animacion, el cursor quedaba fuera de esos circulos y el click caia
        // al suelo (SelectedCharacter = -1): los "clicks que no atacan".  El
        // motivo por el que se habia reemplazado (Collision_SegmentToOBB era un stub que
        // devolvia 1) ya no aplica: quedo portado el 2026-09-04.
```

### Línea 454 en `Entity_SelectNearest` — antes de `const int map = (int)World;`

```cpp
            // Filtro de techos (IDA L~115-131): en Lorencia (World 0) una entidad
            // sobre un tile 4, y en Devias (World 2) sobre un tile 3, solo se
            // puede elegir si el heroe esta en ese mismo tipo de tile.
            // `World` es el indice de mapa (el macro `World` que lo
            // nombraba World mentia; la nota vieja que deshabilito este
            // filtro partia de esa etiqueta).
```

### Línea 497 — antes de `int __cdecl ItemOnGround_HoverTest(void)`

```cpp
// ItemOnGround_HoverTest @ 0x004AFA40
// 2026-09-04: el encabezado decia "NEUTRALIZADO (2026-04-26)", pero eso quedo
// viejo -- la funcion se reimplemento el 2026-07-27 y anda (pickup confirmado en
// runtime).  Se conserva la nota historica porque explica la DESVIACION que sigue
// vigente:
//
//   El path fiel (sub_4AFA40) hace un test de rayo contra la OBB del item con
//   `sub_513260`; aca se usa proximidad world-space -- se compara el tile del item
//   con el tile del terreno bajo el mouse (el mismo picker del click-to-move,
//   RenderTerrain -> DAT_080ab288/28c).
//
//   El motivo que se anotaba para no portarlo ("Collision_SegmentToOBB depende de macros
//   Hex-Rays sin portar") YA NO APLICA: ese test quedo portado el 2026-09-04 al
//   arreglar el pick de objetos interactuables.  Si algun dia el hover de items se
//   comporta distinto al original, ese es el cambio a hacer -- pero hoy funciona y
//   tocarlo es riesgo sin beneficio reportado.
//
// El pool DAT_07e12840 es 1000x0x204; layout por slot (base = pool + i*0x204):
//   base+72   active flag
//   base+424  visible flag (lo setea el render)
//   base+16/20  world X/Y del item
//   base+304/308/312  light color (0.2 normal, 1.5 al hover)
// IDA: FUN_004afa40
```

### Línea 522 en `ItemOnGround_HoverTest` — antes de `BYTE* pool = (BYTE*)&DAT_07e12840[0];`

```cpp
    // 2026-07-27: hover de items en el suelo. El path FIEL (sub_4AFA40) usa un
    // point-in-quad screen-space (Collision_SegmentToOBB, 12-arg) que depende de macros
    // Hex-Rays sin portar. En su lugar usamos proximidad world-space: comparar
    // el tile del item con el tile del terreno bajo el mouse (el mismo picker
    // que usa el click-to-move, RenderTerrain → DAT_080ab288/28c).
    // El pool DAT_07e12840 es 1000×0x204; layout por slot (base = pool+i*0x204):
    //   base+72   active flag
    //   base+424  visible flag (lo setea el render)
    //   base+16/20  world X/Y del item
    //   base+304/308/312  light color (0.2 normal, 1.5 al hover)
```

### Línea 570 — antes de `int __cdecl SpecialObject_HoverTest(void)`

```cpp
// SpecialObject_HoverTest @ 0x004B0240 (sub_4B0240)
// Pick de los objetos "operables" del mundo -- sillas, bancos, barandas y los
// orbes de Noria.  La lista la arma `sub_4FF580` desde `CreateObject` (200
// entradas de 12 bytes en DAT_083A2370: [0] activo, [2] puntero al objeto) y el
// indice que devuelve esta funcion va a `SelectedOperate`, que leen
// `RenderCursor` (para cambiar el cursor) y `Player_InputTick` (para encolar
// MOVEMENT_OPERATE = sentarse / apoyarse / flotar).
//
// Pasada 1: baja la luz de todos los operables visibles a 0.2.
// Pasada 2: el primero cuya OBB (objeto+0x130, la que deja Calc_RenderObject)
//           corte el rayo del mouse se ilumina a 1.5 y se devuelve su indice.
//
// 2026-09-04: estaba NEUTRALIZADO (`return -1`) desde 2026-04-26 porque el port
// original iteraba con el bound absoluto 0x83A2CD0 del binario fuente.  El array
// ya esta bien dimensionado en globals.cpp (0x960 = 200 x 12), asi que se acota
// con `sizeof`.  Mientras estuvo neutralizado NADA del mundo era interactuable.
```

## `src/Local/Text_Data.cpp`

### Línea 86 — antes de `void __cdecl OpenTextData(void)`

```cpp
// DAT_0055a7c4 == 1 in our build (see globals.cpp:1248), so we take the
// binary-file branch.
```

## `src/Math/Math_3D.cpp`

### Línea 149 — antes de `void __cdecl BMD_TransformPosition(void *this_,float *param_1,float *param_2,float *param_`

```cpp
// IDA: TransformPosition (0x004409A0)
// BMD_TransformPosition @ 0x004409a0 — Matrix_TransformPoint (thiscall)
// Transforms param_2 by matrix param_1 (via Vector_Transform).
// If param_4 != 0: applies scale (this->+0x68) + offset (this->+0x6c/70/74).
// Otherwise: pure transform, result in param_3.
//
// BUG-FIX 2026-04-27: el decompile original usaba `float local_c; float local_8;
// float local_4;` como 3 vars separadas y pasaba `&local_c` a Vector_Transform que
// escribe 3 floats contiguos. MSVC no garantiza contigüidad → TPos[1]/[2] iban
// a stack slots non-relacionados → sprite spawn positions basura → glow +9,
// wing FX, particles invisibles porque proyectaban fuera del frustum. Mismo
// patrón ya corregido en Names/RenderLinkObject/Sprite_DrawTexturedQuad.
```

## `src/Math/Math_LegacyAngle.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"
```

## `src/Math/Math_LegacyTransforms.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 40 — antes de `void __cdecl AngleMatrix(float *angles, float (*matrix)[4]) {`

```cpp
// AngleMatrix @ 0x004F9DB0 (50 lines) — Build 3x4 rotation matrix from Euler angles
// Standard Quake/Half-Life convention:
//   angles[0] = PITCH (rotation around Y)
//   angles[1] = YAW   (rotation around Z)
//   angles[2] = ROLL  (rotation around X)
// BUG-FIX (CRÍTICO, 2026-04-20):
//   El port previo intercambiaba las etiquetas: calculaba las entradas con
//   sp=sin(angles[0]), sy=sin(angles[1]), sr=sin(angles[2]) PERO las
//   combinaba como si fueran de un orden distinto (fórmulas no-Quake). El
//   resultado: para rot=(0,0,180) (ships, chars login) producía Rx(180)
//   (patas arriba) en vez de Rz(180) (mirando al revés en pie) → todos los
//   modelos volteados. Re-verificado byte-exact contra Ghidra decompile de
//   0x004F9DB0. Mapeo correcto sP→A[0], sY→A[1], sR→A[2].
```

### Línea 103 — antes de `void __cdecl FaceNormalize(float v1[3], float v2[3], float v3[3], float Normal[3]) {`

```cpp
// IDA: FaceNormalize (0x004FA4D0).  Normal de la cara (v1,v2,v3), normalizada.
// Si el largo es 0 no toca Normal.  2026-09-18: era un stub vacio.
```

### Línea 123

```cpp
// 2026-09-26: aca habia un STUB que devolvia false con el nombre real, mientras
// la implementacion completa estaba bajo el nombre CollisionDetectLineToFace.  Su unico
// consumidor es BMD__CollisionDetectLineToMesh (sub_440BE0), o sea el picking
// por triangulo de los objetos del mundo: con el stub nunca detectaba impacto.
// El comentario del stub ademas decia "0x00440C90 approx", que no es esta
// funcion sino un punto DENTRO de sub_440BE0, su propio caller.
```

## `src/Model/Model_Gates.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// IDA: OpenSounds @ 0x0050F030 — Sound_LoadGameSamples
//
// Precarga los sonidos globales del juego (ids 0..109) via LoadWaveFile
// (LoadWaveFile).  El nombre del archivo viene del port anterior; a pesar de
// llamarse "Gates", esta funcion carga audio, no geometria de puertas.
//
// Firma: LoadWaveFile(id, path, nChannels, b3D)
//
// DESVIACION DELIBERADA (2026-08-17) — nombres de archivo.
// En el binario la mayoria de estos paths estan en coreano (cp949):
// "Data\\Sound\\p걷기(땅).wav", "Data\\Sound\\e타격1.wav", etc.  Nuestro pack de
// assets viene renombrado al ingles (pWalk(Soil).wav, eBlow1.wav, ...), igual
// que los .mp3 — asi que se apunta a los archivos reales.  El comentario al
// final de cada linea conserva el nombre coreano original del binario para
// poder re-cruzarlo.
//
// El mapeo se reconstruyo parseando OpenSounds directamente del `.text` del
// binario original (MD5 eb95ac0785e40a7ad60c9ddb5d8bef34): los literales que
// tenia el port estaban CORRUPTOS — casi todos eran "a\xBD\xBA.wav" /
// "p\xBD\xBA.wav" / "e\xBD\xBA.wav" ("스" repetido como placeholder), o sea
// archivos inexistentes.  Por eso no sonaban pasos, golpes, armas, skills de
// guerrero, magias, gritos, beber pocion ni levantar items: solo funcionaban
// los ~20 ids cuyo nombre ya estaba en ingles en el binario.
//
// NOTA: el orden 82..85 -> sKnightSkill1..4 es la correspondencia secuencial
// entre los 4 ataques de guerrero del binario (내려찍기 / 찌르기 / 올려치기 /
// 돌려치기) y los 4 archivos renombrados.  Lo respalda que el Power Slash
// (case 0x38 de Skills_PacketHandler) usa el id 85 = 돌려치기 = golpe giratorio.
```

## `src/Model/Model_Items.cpp`

### Línea 221 en `Model_LoadPlayerAndItemMeshes` — antes de `if (DAT_0055a7c4 == '\0') {`

```cpp
    // Class2 SMD (only in Korean locale)
    // BUGFIX 2026-04-26: era `i+4 < 7` (=i<3, 2 iter); IDA 0x00506170 línea 57
    // `while (v6 + 3 < 7)` = 3 iter (v6=1,2,3) — sin i=3 nunca se carga
    // HelmClass23/ArmorClass23/etc → ME (cls=10) class-default invisible.
```

### Línea 235 en `Model_LoadPlayerAndItemMeshes` — antes de `for (int i = 1; i+3 < 7; i++) {`

```cpp
    // Class2 BMD (classes 1-3) — same off-by-one fix as SMD path
```

### Línea 298 en `Model_LoadPlayerAndItemMeshes` — antes de `for (int i = 0x12; i-0x12 < 4; i++) {`

```cpp
    // Final slot range (0x12-0x15 suffix) — includes helm only for first 3
    // BUGFIX 2026-04-26: el guard era `i-0x11 < 4` (= i < 0x15), perdía la
    // iteración i=0x15 → ArmorMale21/PantMale21/GloveMale21/BootMale21 (idx
    // 0x2A4/0x2C4/0x2E4/0x304) NO se registraban → MG (class=3) con armor
    // tier=20 (Equipment[2]&0xF=4 + bit40 → +16 = 20) renderizaba sólo la
    // cabeza porque los cinco body slots apuntaban a model_idx vacío. IDA
    // 0x00506170 línea 104: `while (v9 - 18 < 4)` = 4 iteraciones (v9=18..21).
```

### Línea 356 en `Model_LoadPlayerAndItemMeshes` — antes de `*(DWORD*)(A + 532)  = 0x3E99999A;  // 0.30f  accion 33 (16*33 + 4)`

```cpp
            // ── DESVIACION DELIBERADA: accion 33 (2026-09-28) ────────────
            // El bucle de arriba es 1:1 con el binario (verificado en el
            // codigo maquina de 0x5073E0: `mov eax,208 / add eax,16 /
            // cmp eax,528 / mov [ecx+eax-0Ch],esi / jle`), y corta en la
            // accion 32.  La 33 queda SIN PlaySpeed, y no la escribe nadie
            // mas: de las cuatro funciones que tocan `Models + 73368`,
            // SetAttackSpeed cubre 34..54/56..67/81..91, AttackStage solo la
            // 61 y RenderCharacter unicamente lee.
            //
            // Las dos son el par de la montura, y salen del orden de
            // OpenSMDAnimation en esta misma funcion:
            //     11 uniconp_stop.smd          12 uniconp_stop_weapon.smd
            //     32 uniconp_run.smd           33 uniconp_run_weapon.smd
            //     34 attack_fist.smd  <- ancla: SetAttackSpeed escribe el
            //                            offset 548 = 16*34 + 4
            //
            // O sea la 33 es "montado y caminando CON arma equipada".  Como
            // BMD::Open no lee PlaySpeed del archivo (escribe action+8/+10/
            // +12, nunca +4) y `operator_new` no limpia, en el original ese
            // float queda en memoria sin inicializar: no es una decision del
            // binario sino UB, y el resultado depende del allocator.  En
            // release suele ser 0 (paginas frescas del OS) y en nuestro build
            // Debug el relleno del CRT es 0xCDCDCDCD, que como float es
            // negativo y CharacterAnimation lo clampea a 0.  Por los dos
            // caminos el frame no avanza y el jinete queda congelado en el
            // frame 0 mientras la montura si se anima.
            //
            // Medido con sonda (helper=818, arma en LH): `act=33 spd=0.0000
            // f=0.000->0.000 nF=7` — la animacion existe y tiene 7 frames,
            // asi que la intencion era incluirla; el bucle tendria que haber
            // cortado en 544.  Le damos el mismo 0.30f que al resto del
            // bloque de walk/run, y en particular que a su par la 32.
            //
            // Misma clase que el buffer de huesos sin inicializar de
            // CreateCharacterPointer: reproducir la UB no es ser fiel.
            //
            // Sin tocar quedan las otras acciones que ningun writer cubre
            // (0, 55 y 68..77): no hay sintoma reportado ni forma de saber
            // que valor les corresponde.  Si aparece otra animacion congelada
            // del jugador, empezar por ahi.
```

## `src/Model/Model_Misc.cpp`

### Línea 76 en `Model_LoadSkillEffectAssets` — antes de `for (int i = 0xc5; i-0xc5 < 2; i++)`

```cpp
    // 2026-09-02 (Inferno sin fuego / "solo un pedazo del circulo"): estos
    // bucles venian con la BASE de la condicion tomada del argumento en vez del
    // valor inicial.  IDA los escribe asi (OpenSkills 0x0050B710 L91-97):
    //     v1 = 206;
    //     do { AccessModelWithTextures(v1, ..., v1 - 205); ++v1; } while ( v1 - 206 < 3 );
    // o sea la condicion usa el INICIO (206) y el argumento otra base (205).
    // El port usaba la del argumento en los dos lados, asi que cada bucle
    // cargaba (N - (inicio - base)) modelos en vez de N.
    //
    // Efecto medido con la sonda INFERNO: `mdl197=1 mdl198=0`, o sea Stone02
    // nunca se abria.  Effect_SpawnBombRing elige `rand()%2 + 197` en cada una
    // de las 8 posiciones del anillo, asi que ~la mitad de los efectos apuntaba
    // a un modelo vacio: de ahi "solo carga un pedazo del circulo".
    // Habia 17 bucles con el mismo error (ver Model_Players.cpp).
```

## `src/Model/Model_Monsters.cpp`

### Línea 34 en `Model_LoadPlayerEquipmentTextures` — antes de `for (int i = 0x397; i-0x397 < 4; i++) {`

```cpp
    // Class equipment texture binding (slots 0x390-0x3af range, class 1-4)
    // BUGFIX 2026-04-26: era `i-0x396 < 4` (3 iter), IDA 0x00507610 línea 38
    // `while (v0 - 919 < 4)` con v0=919 → 4 iter (919..922). Faltaba ArmorClass4
    // (texturas idx 922/915/929/936/943 = MG class-default).
```

### Línea 46 en `Model_LoadPlayerEquipmentTextures` — antes de `for (int i = 0x290; i-0x290 < 0x11; i++) {`

```cpp
    // Male equipment texture binding (slots 0x270-0x30f range, tiers 1-10 × 5 types)
    // BUGFIX 2026-04-26: era `i-0x28f < 0x11` (16 iter), IDA `v1-656 < 17`
    // → 17 iter (656..672). Faltaba ArmorMale17 (idx 672) — usado por chars
    // tier-17 con bits +16+1 en Equipment[2].
```

### Línea 58 en `Model_LoadPlayerEquipmentTextures` — antes de `for (int i = 0x39b; i-0x39b < 3; i++) {`

```cpp
    // Class2 equipment texture binding (slots 0x394-0x3b3, class2 tiers 1-3 × 5 types)
    // BUGFIX 2026-04-26: era `i-0x396 < 7` (2 iter), IDA `v2-919 < 7` con v2=923
    // → 3 iter (923..925). Faltaba ArmorClass23 = ME (cls=10) class-default.
```

### Línea 69 en `Model_LoadPlayerEquipmentTextures` — antes de `for (int i = 0x2a1; i-0x2a1 < 4; i++) {`

```cpp
    // Elf equipment texture binding (slots 0x280-0x30f, tiers × 5)
    // BUGFIX 2026-04-26: era `i-0x2a0 < 4` (3 iter), IDA `v3-673 < 4` con v3=673
    // → 4 iter (673..676). Faltaba ArmorMale21 (idx 676) — el bug central que
    // dejaba al MG completamente blanco aun con la geometría cargada por el
    // fix paralelo en Model_Items.cpp.
```

## `src/Model/Model_Players.cpp`

### Línea 133 en `Model_LoadItemMeshes` — antes de `for (int i = 400; i-400 < 17; i++)`

```cpp
    // ── BMD compressed asset loads (always run) ───────────────────────────────
    // BUGFIX 2026-04-27: TODOS los loops de items tenían off-by-one Ghidra
    // (`i-(start-1)<N` en vez de `i-start<N`), así que el último índice de cada
    // tipo de arma quedaba sin cargar. SM (Staff 568) y DK (Spear 505) crasheaban
    // silenciosamente al renderizar arma → invisibles. Mismo patrón que los
    // wing/armor loaders que ya fixeamos. Ver IDA 0x005079D0_OpenItems.
    //
    // Swords (0x190-0x1a0) — 17 iter (400-416)
```

### Línea 293 en `Model_LoadItemMeshes` — antes de `for (int i = 0x33e; i-0x33e < 2; i++)`

```cpp
    // BUGFIX 2026-09-01: el bound era `i-0x33d < 2` (base 829) -> UNA sola
    // vuelta, asi que el modelo 0x33f (831 = Fruit, Quest05.bmd) nunca se
    // cargaba: la fruta quedaba invisible en el grid del inventario y por eso
    // no habia nada que hoverear para que saliera su tooltip.
    // IDA 0x5079D0 L203-209:
    //   v30 = 830; do { AccessModelWithTextures(v30, ..., "Quest", v30 - 826); ++v30; }
    //   while (v30 - 830 < 2);      // -> 830 (Quest04) y 831 (Quest05)
```

### Línea 303 en `Model_LoadItemMeshes` — antes de `for (int i = 0x310; i-0x310 < 3; i++)`

```cpp
    // Wings BMD (0x310-0x312)
    // BUGFIX 2026-04-26: era `i-0x30f < 3` → 2 iter (perdía Wing03 idx 0x312).
    // IDA 0x005079D0 línea 216: `while (v31 - 784 < 3)` con v31=784 → 3 iter.
```

### Línea 317 en `Model_LoadItemMeshes` — antes de `for (int i = 0x313; i-0x313 < 4; i++)`

```cpp
    // Wings BMD extended (0x313-0x316) — Wing04..Wing07
    // BUGFIX 2026-04-26: era `i-0x312 < 4` → 3 iter (perdía Wing07 idx 0x316,
    // ala que usa MG/0x316). IDA línea 224: `while (v32 - 787 < 4)` con v32=787 → 4 iter.
```

### Línea 341 en `Model_LoadItemMeshes` — antes de `{`

```cpp
    // NoneBlendMesh flags — RE-HABILITADO 2026-07-16 (fix del filo glowing).
    // Marca el mesh del FILO (emisivo) de estas armas como NoneBlendMesh=1 (mesh+0),
    // que BMD_DrawMesh saltea en el path chrome/oil/blur (línea "if (*pcVar1) return").
    // Efecto: el glow +N chrome NO se pinta sobre el filo (que conserva su luz propia);
    // va al hilt/mango. Sin esto, el chrome cubría el filo → "barra dorada".
    //
    // Fiel a IDA OpenItems (0x005079D0 L245-249). Los offsets son BYTES dentro del
    // array de modelos (Models = DAT_05828d58, stride 0xBC, campo Meshs en +0x28), NO
    // direcciones absolutas ni índices ×4 — una sesión previa les agregó un ×4 espurio
    // y comentó todo. `Models + 0x16c68` = Model[496].Meshs; `+0x28` = mesh[1].
    //   0x16c68 = 496*0xBC+0x28 → Light Spear (MODEL_SPEAR)    mesh[1]
    //   0x12d40 = 410*0xBC+0x28 → Light Saber (MODEL_SWORD+10) mesh[1]
    //   0x19fd0 = 566*0xBC+0x28 → Staff+6                      mesh[2] (+0x50)
    //   0x15950 = 470*0xBC+0x28 → Mace+6                       mesh[1]
    //   0x2be38 = 956*0xBC+0x28 → Event+9                      mesh[1]
```

## `src/Party/Party.cpp`

### Línea 278 — antes de `void Guild_CreateOk(BYTE* pkt)`

```cpp
// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  CÓDIGO MUERTO — desde 2026-08-26 no tienen callers                      ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// Las siete funciones `Guild_*` que siguen hasta el final del archivo estaban
// enganchadas a los opcodes 0x90-0x99. Ese rango NO es guild: IDA y MuEmu
// coinciden en que son eventos (Devil Square, Blood Castle, Golden Archer).
// La tabla opcode -> función real está en la cabecera de
// `src/Net/Net_Events.cpp`, que es quien los atiende ahora.
//
// El guild de verdad usa 0x50-0x56 y ya estaba bien atendido en Net_Process.cpp
// (`ReceiveGuildResult`, `ReceiveGuildList`, `ReceiveCreateGuildResult`, ...);
// esta tanda no lo tocó.
//
// No se borran todavía porque el cuerpo puede servir de referencia si alguna
// vez se porta el protocolo de guild de otra versión. Ojo con reengancharlas:
// `Guild_CreateOk` ENVÍA un `[C1][03][31]`, así que colgada del opcode
// equivocado no sólo muestra un cartel de más, también le manda basura al
// server.
//
// ============================================================
// Guild_CreateOk  @ 0x00436820  (opcode 0x90)
// Server ACKs guild creation; displays result message.
//
// Sends back a 3-byte packet [C1][03][31] (guild creation ACK).
// Then switches on pkt[3] (sub-type 1-5) to pick a pre-loaded
// string buffer and calls CreateOkMessageBox to display it in chat.
// Standard WSAEWOULDBLOCK retry loop for the send().
// ============================================================
```

## `src/Path/Path_Legacy.cpp`

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

### Línea 48 — antes de `static int PF_AStar(int sx, int sy, int tx, int ty, int iWall, bool bErrorCheck,`

```cpp
// ── PF_AStar — sustituto del PATH::FindPath original (0x0043F500) ────────────
//
// 2026-08-17 — CRITERIO DE BLOQUEO CORREGIDO CONTRA EL BINARIO.
//
// El A* de este port decidía el bloqueo con una MÁSCARA DE BITS inventada:
//     return (a & 0x0e) != 0;      // "mask correcto verificado contra Terrain1.att"
// El original NO usa máscara. `PATH::FindPath` @ 0x0043F500 hace una COMPARACIÓN
// NUMÉRICA contra el parámetro `iWall`, restando antes el bit 0x20 si está puesto:
//
//     uVar5 = TerrainWall[idx];
//     if ((TerrainWall[idx] & 0x20) == 0x20) uVar5 - 0x20;
//     if (((visited[idx] & 1) == 0) && ((int)uVar5 < iWall)) { ...expandir vecino... }
//
// Con iWall=2 ambos criterios coinciden para los attrs comunes (0,1 pasan; 2,3,4,5
// bloquean), y por eso el bug pasó desapercibido — pero DIVERGEN en los bits altos:
// 0x10, 0x40 y 0x80 pasan la máscara 0x0e (`a & 0x0e == 0` → "libre") y en cambio
// el original los bloquea (0x10 < 2 es falso). De ahí que el héroe caminara por
// encima de terreno prohibido.
//
// El chequeo del destino cuando bErrorCheck=true también es del binario, y ahí el
// attr va CRUDO (sin restar 0x20), exigiendo que el bit 0x20 no esté puesto:
//     if ((bErrorCheck) && (iWall <= TerrainWall[dst]) &&
//         ((TerrainWall[dst] & 0x20) != 0x20)) return false;
//
// Con bErrorCheck=false el original no falla si no alcanza el destino: se queda con
// el mejor nodo alcanzado (`local_18`/`local_14`) y devuelve el camino hasta ahí.
// La métrica de "mejor" es la del propio binario:
//     dx=|x-xEnd|, dy=|y-yEnd|; m = (dx==1 && dy==1) ? 0 : min(dx,dy);
//     coste = (|dx-dy| * 0xf + 3 + m * 0x15) >> 2;
//
// Se elimina de paso la búsqueda de "walkable más cercano en radio 3" que había
// aquí: era invención del port. El original falla y deja que PathFinding2 reintente
// con otro iWall, que es lo que se replica en Path_FindRoute.
//
// 2026-08-17 (b) — `fDistance` (el `radius` de PathFinding2) TAMPOCO se usaba.
// En el binario ese parámetro llega a FindPath como `Value` y parte la función en
// dos ramas bien distintas:
//     if (_Value == 0.0) { ...un único destino: marca[dst] = 4... }
//     else               { ...recorre el DISCO de radio Value alrededor del destino
//                            y marca = 4 toda celda con SQRT(dx*dx+dy*dy) < Value... }
// y la búsqueda termina al alcanzar CUALQUIER celda marcada con 4. O sea: con
// fDistance > 0 basta con acercarse al destino, no hace falta pisarlo.
// El combate (Combat.cpp:1053 y siguientes) llama siempre con `skillRange`, así que
// ignorar el parámetro hacía que el héroe intentara pisar la casilla exacta del
// objetivo — la que suele estar ocupada por el propio mob. Nótese además que el
// chequeo estricto del destino sólo existe en la rama `_Value == 0.0`.
```

### Línea 110 en `PF_AStar` — antes de `auto origCost = [&](int x, int y) -> int {`

```cpp
    // Métrica de cercanía al destino del original (para el mejor esfuerzo).
    // IDA sub_43F500 (L~226-236 del raw):
    //   v56 = |x - tx|; v57 = |y - ty|;
    //   if (v56 == 1 && v57 == 1) { v57 = 0; v58 = v57; }   // diagonal pegada
    //   else v58 = min(v56, v57);
    //   costo = (15 * |v56 - v57| + 21 * v58 + 3) / 4;
    // El `v57 = 0` se hace ANTES de |v56 - v57|, asi que una celda pegada en
    // diagonal cuesta 4, igual que una pegada en recto.  El port calculaba la
    // diferencia con el dy original (0) y le daba costo 0: con el mob pegado en
    // linea recta una celda en diagonal le ganaba al origen, se armaba un paso
    // y el heroe caminaba en vez de atacar.
```

### Línea 151 en `PF_AStar` — antes de `const int MAX_NODES = bErrorCheck ? 500 : 50;`

```cpp
    // 2026-08-17 — tope de iteraciones tomado del binario. FindPath hace:
    //     iVar10 = (-(uint)(bErrorCheck != false) & 0x1c2) + 0x32;
    // o sea 0x1c2+0x32 = 500 con bErrorCheck, y 0x32 = 50 sin él. Acá había 4096
    // fijo para ambos: contra una pared el A* barría 4096 nodos POR TICK
    // (visible en debug.log como "explored=4096" repetido).
```

### Línea 254 en `PF_AStar` — antes de `return 0;`

```cpp
        // 2026-08-17 (c) — NO tocar `path` al fallar. Ver nota de abajo.
```

### Línea 258 en `PF_AStar` — antes de `static unsigned char wpX[4096], wpY[4096];`

```cpp
    // Backtrack COMPLETO destino → origen (ver BUG-FIX del truncado, abajo el
    // bucle de emisión se queda con los 15 primeros contados desde el origen).
```

### Línea 275 en `PF_AStar` — antes de `unsigned char tmpX[16], tmpY[16];`

```cpp
    // 2026-08-17 (c) — CAUSA RAÍZ de "atraviesa la pared y no para".
    //
    // Este bloque escribía directo sobre `path` (= entidad+0x354) y, en los
    // caminos de fallo, lo dejaba en cero: `path[0]=0; path[1]=0; path[2]=0;`.
    // path[2] es PATH_t.PathNum — o sea que un pathfind fallido BORRABA el
    // camino que la entidad ya venía siguiendo, sin tocar Movement (+0x2EC).
    //
    // Y ese estado (Movement=1, PathNum=0) es una deriva infinita, también en el
    // original: MovePath @ 0x0043EA20 abre con
    //     if ((c->Path).PathNum <= (c->Path).CurrentPath) return false;
    // y MoveHero, ante ese false, llama MoveCharacterPosition, que avanza la
    // posición en línea recta según el Angle actual — sin mirar terreno ni path.
    // Nunca se alcanza el `if (MovePath(...))` que hace Movement=0 + SetPlayerStop.
    //
    // El original NUNCA cae ahí porque PathFinding2 @ 0x0043F3E0 no toca el buffer
    // cuando falla: `a` sólo se escribe pasado LAB_0043f483, en la rama de éxito;
    // los dos `return false` salen con el camino anterior intacto.
    //
    // Se hizo visible al arreglar el hold: el recálculo por tick contra una pared
    // falla una y otra vez, y cada fallo borraba el PathNum del camino en curso.
    // Evidencia en debug.log: `wp=0/2 2ec=1` y al tick siguiente `wp=0/0 2ec=1`,
    // con la posición avanzando en línea recta a paso constante.
    //
    // Ahora se arma todo en un temporal y `path` sólo se escribe si hay éxito.
```

### Línea 333 en `Path_FindRoute` — antes de `void* pfCtx = (void*)(intptr_t)DAT_05826df4;`

```cpp
    // BUG-FIX 2026-04-26 (audit #4): _this debe ser el contexto del pathfinder
    // (DAT_05826df4), no `sx`. Net_Process.cpp documenta el wrapper:
    //   PATH_FindPath(DAT_05826df4, id, t, x, y, 1, 2, t)
    // Antes pasábamos `(void*)sx` → la función deref-eaba un coord como ptr.
```

### Línea 339 en `Path_FindRoute` — antes de `#define PF_USE_ORIGINAL 0`

```cpp
    // 2026-08-17: el contexto YA se construye completo. Antes se reservaba en
    // WinMain con `malloc(0x420)` + memset y el vtable de la cola de prioridad
    // (+0x414) quedaba NULL, así que PATH_FindPath (PATH::FindPath) crasheaba al
    // dereferenciarlo — de ahí el `pfReady = false` forzado desde 2026-05-03.
    // Ahora PathContext_Create() (src/Game/PathFinder.cpp, llamada desde WinMain)
    // replica el ctor del binario: 0x0043F280..0x0043F2C7, reserva de 0x424 bytes
    // -no 0x420- y vtable en +0x414. InitPath (PathFinder_ResetContext, mas abajo en este
    // mismo archivo) ya estaba portada y la llama OpenFont (World_Init), igual
    // que en el binario; corre despues del ctor, que es el orden correcto.
    //
    // El camino original queda detrás de un switch porque PATH_FindPath todavía
    // no se ejercitó en runtime: nuestro A* sustituto sigue siendo el default.
    // Poner PF_USE_ORIGINAL en 1 para usar el algoritmo del binario.
    // 2026-08-17: probado en runtime con 1 → CRASH inmediato en la primera llamada
    // (0xC0000005 leyendo 0x63082BFC). El contexto se construye bien -el log
    // muestra `pfCtx check #1: vtbl@0x414=0x6960b4 pfReady=1`-, asi que el ctor
    // esta ok y el problema esta dentro de la propia portacion de PATH_FindPath.
    // Queda en 0 hasta auditar esa funcion contra el decompile. Ver DESCOBERTAS.md.
```

### Línea 373 en `Path_FindRoute` — antes de `//`

```cpp
        // Replica de ZzzAI::PathFinding2 @ 0x0043F3E0, con nuestro A* (PF_AStar)
        // en lugar de PATH::FindPath (0x0043F500), cuyo ctor de PriorityQueue no
        // esta portado.  El original hace:
```

### Línea 414 en `Path_FindRoute` — antes de `if ((DAT_0838bc70[srcAttr] & 1) == 1 || (DAT_0838bc70[dstAttr] & 1) == 1) {`

```cpp
        // 2026-08-17: estaba INVERTIDO respecto del binario. En PathFinding2 el
        // filtro sube a 4 cuando origen/destino tienen el bit 0, y vuelve a 2 sólo
        // si el destino tiene además el bit 1:
        //     iVar3 = 4;  if ((TerrainWall[dst] & 2) == 2) iVar3 = local_4 /*2*/;
        // Acá se hacía al revés (subía a 4 justo cuando el bit 1 estaba puesto).
        // Camino muerto hoy (pfReady siempre false), pero queda alineado.
```

### Línea 452 — antes de `float __cdecl FarAngle(float a1, float a2, char a3)`

```cpp
// IDA: FUN_0043e370 (0x0043E370)
// Returns signed angular difference between two angles, handling wrap-around at 360.
// If mode==1, returns absolute value (unsigned distance).
//
// BUG-FIX 2026-04-26 (audit #1): la decompilación IDA emitió `if (v6)` con `v6`
// como flag FPU x87 (`c0`) sin reconstruir → undefined branch. El asm hace
// `fcom a2, a1` (comparando a2 con a1) ANTES del `fsub` → v6 representa
// `a2 > a1`, no el signo del result. Reescrito preservando exactamente las
// asignaciones del decomp (`360 - a2 + a1` y `360 - a1 + a2`) en cada rama.
```

### Línea 656 — antes de `char __cdecl Path_IsLineClear(int sx1, int sy1, int sx2, int sy2) {`

```cpp
// IDA: CheckWall @ 0x004830B0 — bool __cdecl CheckWall(int sx1, int sy1, int sx2, int sy2)
// Bresenham desde (sx1,sy1) hasta (sx2,sy2) sobre TerrainWall (DAT_0838bc70);
// devuelve 1 si la linea esta despejada, 0 si topa con un tile bloqueante.
// Nombres: tile = v4 · err = v5 · dx = v6 · dy = v7 · xStep = v8 · yStep = v9
//          major = v10 · minorDelta = y · minorInc = v11 · steps = x
//
// 2026-09-01 FIX: el port tenia `major` y `minorDelta` INTERCAMBIADOS respecto
// de IDA en las dos ramas del if (los dos incrementos si estaban bien).  El
// efecto era brutal en lineas casi axiales: con dy == 0 quedaba major == 0, o
// sea el bucle probaba UN solo tile en vez de |dx|+1 y la funcion devolvia 1
// casi siempre.  La usan Attack (3 sitios, uno de ellos el gate del bucle de
// manos), Action (2) y Player_InputTick (1).
```

### Línea 702

```cpp
// Net / packet — all use HashTable obfuscation; stubs preserve observable side effects.
```

## `src/Path/Path_LegacyReset.cpp`

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

## `src/Physics/Cloth_Simulation.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.
```

### Línea 156 — antes de `float __fastcall SpringNode_Delta(void *a, int b, float *c);          // delta + |delta|`

```cpp
// ═══════════════════════════════════════════════════════════════════════════
// Sistema de tela (capa del MG) — SOLVER. Port 2026-08-11.
//
// Layout del widget (stride 0x54), índices en DWORDs como los usa IDA:
//   [1]=+0x04 entity   [2]=+0x08 boneIdx   [5]=+0x14 flags
//   [10]=+0x28 W       [11]=+0x2C H        [12]=+0x30 nodeCount
//   [13]=+0x34 nodes*  [14]=+0x38 springCount  [15]=+0x3C springs*
//   [18]=+0x48 anchorCount  [19]=+0x4C head sentinel  [20]=+0x50 tail sentinel
// Nodo (stride 60 = 0x3C):
//   +0x00 vtable  +0x04..0x0C accel  +0x10..0x18 vel  +0x1C..0x24 pos
//   +0x28 pinned(byte)  +0x2C corrCount  +0x30..0x38 corrAccum
// Spring (stride 16): [short na][short nb][float rest][byte flags]
//   flags&1 = equality (sub_407C60)   flags&2 = wind/stretch (sub_408CB0)
//   flags&4 = rango min/max (sub_407B90)
// Ancla (stride 0x24): +0x00 vtable  +0x04..0x0C localPos  +0x10 boneIdx
//   +0x14..0x1C worldPos  +0x20 radio
//
// NOTA sobre el `double a2@<st0>` de los decompiles: es un artefacto. En el
// disasm (0x407C71, 0x407C76) el valor comparado contra 0.001 lo PRODUCE
// `sub_407B50` en ST0 — Hex-Rays no modela ese retorno y lo atribuye a un
// parámetro de entrada. Los ports reconstruyen la distancia real.
// ═══════════════════════════════════════════════════════════════════════════
```

### Línea 184 — antes de `float __cdecl Vec3_Length(float *a1)`

```cpp
// Vec3_Length @ 0x004f9c40 — Vec3_Length: returns sqrt(dot(v,v)), does NOT modify v.
// (Ghidra shows return as float10 left in x87 ST0; callers use the return value as distance.)
// Vec3_Length (IDA-activated, was Ghidra stub)
```

### Línea 215 — antes de `void __fastcall ClothNode_Integrate(void *a, float v) {`

```cpp
// 2026-08-11: el port limpiaba la aceleración al final. Eso NO está en IDA —
// la aceleración la reescribe entera `sub_4079E0` (gravedad + viento) al
// principio de cada tick, así que el clear extra era inofensivo pero falso.
```

### Línea 352 — antes de `char __fastcall VerletSystem_Flush(int a1)`

```cpp
// VerletSystem_Flush @ 0x00407d10 — VerletSystem_Flush: apply accumulated position corrections, zero buffer
// VerletSystem_Flush (IDA-activated, was Ghidra stub)
```

### Línea 431 — antes de `void __fastcall SpringNode_Ctor(void *_this)`

```cpp
// SpringNode_Ctor @ 0x00407950 — SpringNode_Ctor: set vtable + zero fields.
// SpringNode_Ctor (IDA-activated, was Ghidra stub)
```

### Línea 491 — antes de `int __cdecl VerletNode_CtorExt(DWORD *_this)`

```cpp
// VerletNode_CtorExt @ 0x00407ED0 — VerletNode_CtorExt: zero fields + clear +0x20.
// VerletNode_CtorExt (IDA-activated, was Ghidra stub)
```

### Línea 529 — antes de `DWORD *__cdecl ClothAnchor_Ctor(DWORD *_this)`

```cpp
// ClothAnchor_Ctor @ 0x00407E50 — ClothAnchor_Ctor: full constructor (base + ext).
// ClothAnchor_Ctor (IDA-activated, was Ghidra stub)
```

### Línea 622 — antes de `void __fastcall ClothAnchor_SetParams(void *node, float p1, float p2, float p3, float radi`

```cpp
// ClothAnchor_SetParams @ 0x00407EF0 — ClothAnchor_SetParams.
// Fields: +4/+8/+0xc = posición LOCAL, +0x20 = radio, +0x10 = índice de HUESO.
//
// 2026-08-11: el último parámetro era `float`. En IDA (`sub_407EF0`) es
// `this[4] = a6` — un DWORD entero, y `sub_408E30` lo usa como
// `48 * v5[4]` para indexar la matriz de huesos. Con 17.0f guardado como
// float, `v5[4]` valía 0x41880000 y el índice se iba a 52 GB del arranque
// de la tabla. Los call sites de IDA lo confirman: los 5 primeros args son
// bits de float y el 6º un entero chico (2, 10, 17, 18, 19).
```

## `src/Trade/Trade.cpp`

### Línea 106 — antes de `#define ENTITY(idx)  ((BYTE*)DAT_07abf5d0 + (idx) * 0x394)`

```cpp
// 2026-05-07: g_EntityBase is never wired to the actual entity array — the real
// base lives in DAT_07abf5d0 (set by WinMain). Use that directly so the ENTITY
// macro doesn't yield a NULL deref.
```

### Línea 241 en `Shop_EntitySlots` — antes de `SHORT slot = -1;`

```cpp
        // BUG-FIX 2026-05-03: shop table is at literal `0x07e919b8` (unmapped in
        // our build) and the bound `0x7ea51e8` is also a literal. Until the
        // 1238-slot shop table is properly allocated, leave slot = -1 (no match).
```

### Línea 290 en `PacketHandler_0x5d` — antes de `DAT_07eaa114 = 0;`

```cpp
    // BUG-FIX 2026-05-03: was writing to literal source-binary addresses
    // 0x07eaa114 and 0x07eaa0d0 — random memory in our build. Use the symbols
    // that the linker actually placed those values at.
```

### Línea 371 en `LegacyMisclassified_TradeRequestResult` — antes de `static const char* msg_table[] = {`

```cpp
    // BUG-FIX 2026-05-03: previous tables held literal source-binary addresses
    // (msg_table 0x07d4fd58.., label_table 0x05826dc8..) — unmapped in our
    // build → AV the moment a 0x60 response arrived. Until the localized text
    // pool is wired through GlobalText[], use ASCII placeholders.
```

### Línea 452 en `LegacyMisclassified_TradeOpen` — antes de `char window_title[100];`

```cpp
    // BUG-FIX 2026-05-03: function originally read format strings from absolute
    // source-binary addresses (`(char*)0x07d5058c`, `(char*)0x07d50dc0`) that are
    // unmapped memory in our build, and walked a shop table at literal
    // `0x07e919bc` (also unmapped). Both would AV the moment a trade/duel
    // packet arrived. Until proper format-string globals are added and the
    // shop table is properly allocated, fall back to plain ASCII titles +
    // skip the slot scan so the rest of the trade UI can still open.
```

### Línea 521 en `LegacyMisclassified_TradeItemUpdate` — antes de `static const struct { const char* str; int ack; } result_table[] = {`

```cpp
    // BUG-FIX 2026-05-03: result_table held literal source-binary addresses
    // (0x07d506b8..0x07d4cfa4) for result strings — unmapped in our build.
    // Replace with ASCII placeholders until proper text-pool wiring exists.
```

## `src/Util/Misc.cpp`

### Línea 180 — antes de `void MoveItems(void)`

```cpp
// IDA: MoveItems (0x00503760)
// Iterates the entity-gravity pool (per-slot offset +0x18 inside the
// 1000-slot ground-items pool DAT_07e12840, stride 0x204). Per active slot:
// advances Z by velocity, decays velocity by _DAT_005527d0. Checks terrain
// height via RequestTerrainHeight; if entity is above terrain + offset, adjusts Y or
// Z velocity. Calls ItemAngle and Entity_UpdateSparkleEffect (FUN_00503650).
//
// 2026-05-08: AUTO-SKIP removed. Walker now uses the properly-sized pool
// `DAT_07e12840` (1000 × 0x204) with an explicit slot count instead of the
// literal end-bound `< 0x7e907f8`. Per-slot pfVar2 = slot_base + 0x18 (the
// gravity-field anchor that the orphan DAT_07e12858 used to alias).
```

### Línea 193 en `MoveItems` — antes de `for (int slotIdx = 0; slotIdx < 1000; ++slotIdx) {`

```cpp
  // 2026-08-21: el walker estaba corrido 72 bytes.  Tomaba `DAT_07e12840` como
  // si fuera `Items + 72` (leía el flag activo en slot+0), pero en nuestro build
  // ese símbolo ES la base del item — es lo que asumen Net_Process (0x20) y
  // Entity_Render (que escriben/leen active en ip+72).  Resultado: el flag activo
  // salía siempre 0 y la función no hacía NADA: los items no caían al suelo, no
  // giraban al caer y no soltaban destellos.
  //
  // IDA MoveItems (0x503760) trabaja sobre `v0 = &Items[0][96]` (la Z), así que
  // los offsets equivalentes desde la base del item son:
  //   ip+72  active   ·  ip+74  modelo  ·  ip+88/92  X,Y  ·  ip+96  Z
  //   ip+100/104  Angle[0]/Angle[1]     ·  ip+288  velocidad Z
```

## `src/Util/Process_Exit.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```
