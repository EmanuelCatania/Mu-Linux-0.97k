# Historial de comentarios: `src/Core/`

Comentarios de desarrollo movidos desde `src/Core/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en el tag `0.97.00`.

## `src/Core/Legacy_Runtime.cpp`

### cabecera del archivo (línea 2)

```cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

## `src/Core/Runtime_Externs.cpp`

### cabecera del archivo (línea 2)

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### declaraciones de Cloth_Integrate / Cloth_Solve (línea 10)

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### ChatListBox_ScrollByN (línea 64)

```cpp
// FIX 2026-07-20 — CRASH 0xC0000005 con param0=8 (violación de EJECUCIÓN):
// las 4 ramas hacían `(**(void(__cdecl**)(int))(*(int*)param_1 + 0x30))(0)`.
// `*param_1 + 0x30` es vtable+48 = entrada 12 (sub_40CC50 / scrollByN), que es
// __thiscall.  Al invocarla como __cdecl con un solo argumento, el `this` no
// viajaba en ECX: la callee tomaba como `this` la basura que hubiera quedado en
// ECX, deferenciaba su "vtable" y saltaba a una dirección arbitraria.
// El disasm (0x40E35D, 0x40E375, 0x40E39C, 0x40E3BE) muestra las 4 ramas como
// `mov eax,[ecx] / push 0 / call [eax+30h]` con ECX intacto = __thiscall(this, 0).
// Hex-Rays tipó UNA de las ramas como __stdcall sin this (perdió el tracking de
// ECX al hoistear `v2 = *this`); las otras tres sí salen como __thiscall.
```

## `src/Core/Runtime_Linker.cpp`

### cabecera del archivo (línea 1)

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### SetPlayerStop (comentario sin código) (línea 41)

```cpp
// SetPlayerStop @ 0x004430C0 (504 lines) — Set player entity to idle/stop animation
// Selects animation based on equipment, class, terrain. Most bulk is anti-tamper hash ops.
// 2026-08-08 BUG-FIX (el MG se renderizaba como Dark Wizard, con casco y con
// rayas): este stub coexistía con el port REAL de SetPlayerStop
// (`SetPlayerStop`, Net/SecondPassword.cpp). `Character_UpdateEquipSlotAnimations` llamaba a ESTE, y el
// stub hacía:
//     *(BYTE*)(entity + 0x1bc) &= ~0x07;   // "clear movement bits"
// pero **0x1BC NO son move flags: es el byte de CLASE/skin** (lo leen
// `SetCharacterClass` como `skin`, `CheckFullSet` como `(c+444)&7`, y
// `RenderEquipmentBox` vía `CA[11]`). O sea el stub borraba la clase:
//     DW  0x00 -> 0x00   (sin cambio, por eso nunca se notó)
//     SM  0x08 -> 0x08   (sin cambio)
//     DK  0x01 -> 0x00   ✗ pasa a Dark Wizard
//     FE  0x02 -> 0x00   ✗
//     MG  0x03 -> 0x00   ✗
// Cazado con las sondas CLSPROBE: F(post-45c130)=3 → G(post-45c720)=0.
// Delegamos al port real; el stub no debe existir.
```

### StopBuffer (referencia) (línea 87)

```cpp
// StopBuffer @ 0x00404C60 — real implementation at stubs.cpp:275 (forwards to FUN_00404c60).
```

### StopMp3 (línea 90)

```cpp
//
// Esta era una SEGUNDA implementacion del mismo simbolo del binario, y es la que
// usaba StopMusic. Estaba mal en tres cosas: ignoraba `cmd` (cerraba el
// reproductor aunque estuviera sonando otro track), mandaba WM_DESTROY en vez de
// WM_CLOSE, y no limpiaba Mp3FileName — asi que el siguiente PlayMp3 creia que
// el track viejo seguia en curso. Ver [[simbolo-duplicado-patron]].
```

## `src/Core/Runtime_Medium.cpp`

### cabecera del archivo (línea 2)

```cpp
//
// Extracted from stubs_bulk_med.cpp (B3: stubs.cpp lines 14828-15881).
```

### GetMapName (línea 62)

```cpp
// (Antes era una tabla de nombres en ingles inventada: el original devolvia
//  punteros a GlobalText y el port lo habia leido como direcciones fijas.)
```

### OpenMacro (línea 91)

```cpp
// OpenMacro @ 0x0050F750 (72 bytes) -- carga Data\Macro.txt
//
// BUG-FIX 2026-04-28: usaba direccion absoluta 0x07e0ffc8 con bound
// 0x07e109c8.  Ahora indexa el array MacroText[10][0x100].
//
// 2026-09-24: el modo era "rb"; IDA abre con "rt" (aRt).
//
// DESVIACION DOCUMENTADA (tomada del DLL, CPatchs::MyOpenMacro): el original
// lee con `fscanf(fp, "%s", slot)`, que **corta en el primer espacio**, asi que
// una macro con mas de una palabra se pierde al reiniciar el cliente aunque
// SaveMacro la haya escrito entera.  El DLL de inyeccion reemplaza esta misma
// funcion por una con `fgets` + recorte del salto de linea; se porta esa
// version, que es la unica que hace util al sistema de macros.  Tambien limpia
// el array antes de leer, como el DLL.
```

### RenderTerrainAlphaBitmaps (línea 394)

```cpp
// RenderTerrainAlphaBitmaps (IDA-activated, was Ghidra stub)
```

### OpenSMDFile (línea 449)

```cpp
// OpenSMDFile @ 0x0040B200 (106 bytes) — open and parse SMD model file
// CRITICAL 2026-05-03: ParseNodes/ParseSkeleton/ParseTriangles are EMPTY STUBS
// (lines 19407, 19417, 19432). If we open SMDFile here and call them, the file
// content is never consumed; fclose() leaves the global SMDFile pointing to a
// freed FILE* — any later reader (GetToken from Monster_Data, etc.) crashes
// dereferencing it. Until the SMD parsers are actually implemented, do not
// touch the SMDFile global. Return false so the SMD chain stays a no-op
// (matches the original behaviour: 0.97k ships only .bmd, no .smd files).
```

### BSTIterator_PostIncrement (línea 509)

```cpp
// BSTIterator_PostIncrement (IDA-activated, was Ghidra stub)
```

### Pool_AllocNextSlot (línea 643)

```cpp
// Pool_AllocNextSlot (IDA-activated, was Ghidra stub)
```

### CSQuest_FindQuestItemsInInven (cabecera) (línea 682)

```cpp
// 0x11 ints = 68 bytes per outer step). Iterates 7 outer × 8 inner = 56 cells.
//
// BUG-FIX 2026-05-03: original port used `if (piVar4 < 0x7ea9328)` — a hardcoded
// absolute bound from the source binary. In our build &DAT_07ea9504 lives at
// a different address (linker-placed) so the comparison was meaningless: it
// either triggered immediately (early-exit returns wrong shortage) or never
// (infinite loop / heap walk crash). Replaced with explicit iteration count.
```

### CSQuest_FindQuestItemsInInven (bucle) (línea 693)

```cpp
    // 2026-08-22: eran 7 columnas y son 8.  El bound de IDA es
    // `while (v5 >= &unk_7EA9328)` arrancando en &unk_7EA9504 con paso de -17
    // ints (-68 bytes): (0x7EA9504 - 0x7EA9328) / 68 + 1 = 8.  Con 7 se salteaba
    // una columna entera del inventario, asi que un item de quest que estuviera
    // ahi contaba como faltante: la lista salia en rojo y el boton en gris
    // aunque el personaje lo tuviera.
```

### forward decls vacías antes de END BATCH 9 (línea 744)

```cpp
// Forward decl for FUN_0052f4d0

// Forward decl
```

### FUN_0052f4d0 (línea 752)

```cpp
// === FUN_0052f4d0 — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
```

## `src/Core/Runtime_Small.cpp`

### cabecera del archivo (línea 1)

```cpp
// Extracted from stubs_bulk_small.cpp (B3: stubs.cpp lines 14828-16129).
//
// Decompiled from Ghidra in bulk — closes ~3KB of the binary gap.
// ~80 functions implemented (1-52 bytes range).
```

### encabezado del BATCH (línea 6)

```cpp
// Decompiled from Ghidra in bulk — closes ~3KB of the binary gap
```

### forward declarations (línea 32)

```cpp
static void __cdecl FUN_00405320_impl(int param_1);
// (FUN_00405290_impl_fwd: forward decl muerto removido — la def real es
//  FUN_00405290_impl @ ~line 13553, __cdecl, sin necesidad de fwd separado)
```

### FUN_0053ad80 (referencia) (línea 38)

```cpp
// FUN_0053ad80 — see proper definition at ~line 15177 (codec noop start_pass)
```

### FUN_00407970 (línea 53)

```cpp
// FUN_00407970 (IDA-activated, was Ghidra stub)
```

### FUN_00407de0 (línea 62)

```cpp
// FUN_00407de0 (IDA-activated, was Ghidra stub)
```

### FUN_00407ec0 (línea 89)

```cpp
// FUN_00407ec0 (IDA-activated, was Ghidra stub)
```

### ClearNotice (línea 141)

```cpp
// ClearNotice @ 0x0047FAC0 (17 bytes) — zero-fill notice text array
// 2026-09-16: borraba DAT_083a2370 + 0x10, que es la lista de objetos
// interactuables (Operates).  Notice vive en 0x07DB80D8 (= DAT_07db80d8, 6 x 0x108).
```

### Stats_CalcBase (guarda de a1) (línea 268)

```cpp
    // GUARDA 2026-07-19 (CRASH 0xC0000005 @ +0xA3): se validaba `ca`
    // (CharacterAttribute) pero NO `a1` (CharacterMachine). Abajo se hace
    // `*(short*)(a1 + 536)` / `(a1 + 604)` (slots de arma) sin chequear, así que
    // con a1 nulo o basura reventaba. Se disparaba al abrir el inventario /
    // mostrar el tooltip de un item, que fuerza un recálculo de stats.
```

### Stats_CalcBase (ItemAttribute_Base) (línea 280)

```cpp
    // GUARDA 2026-07-20 (CRASH 0xC0000005 read @ 0x612A = 388*0x40 + 42):
    // esta funcion indexaba `ItemAttribute[tipo]` leyendo DAT_07d78068 CRUDO,
    // ignorando el helper ItemAttribute_Base() de globals.h que existe
    // justamente porque ese puntero se pisa a 0x1 en runtime (ver la nota de
    // 2026-05-08 sobre tooltip / RenderBrokenItem). Con la tabla en 0, el
    // indexado daba una direccion chica y reventaba.
    // Se disparaba al cerrar el inventario con un tooltip de item abierto,
    // porque ese camino fuerza un recalculo de stats.
```

### FUN_004cbdd0 (línea 506)

```cpp
// BUG-FIX 2026-05-03: original port used absolute source-binary addresses
// (`0x07e11fb0` literal start, `0x7e12830` literal end) — in our build the
// linker places `&DAT_07e11fb0` somewhere completely different so the
// dereference and bound were both garbage.
```

### StopMusic (línea 632)

```cpp
// El bound `< 0x5615DC` es &g_lpszMp3[6], asi que son 6 iteraciones. Antes esto
// era un StopMp3 unico "best-effort" porque g_lpszMp3 era un DWORD = 0 y no
// habia tabla; ahora la tabla existe (ver globals.cpp) y el loop es el de IDA.
```

### MoveBlurs (línea 767)

```cpp
// 2026-05-03: AUTO-SKIP removed. The blur/joint/trail shared pool is now
// allocated as `g_RenderPool_07c608a8[100 * 0x2f0]` in globals.cpp.
```

### SetMatchInfo (línea 795)

```cpp
// 2026-09-04 FIX: la segunda linea escribia `m_iMatchTimeMax` otra vez, con el
// comentario "double-assign (original code bug)".  No hay tal bug -- IDA dice
//     m_byMatchType = byType; m_iMatchTimeMax = iMaxTime;
//     m_iMatchTime  = iTime;  m_iMaxKillMonster = iMaxMonster;
//     m_iKillMonster = iKillMonster;
// `m_iMatchTime` es un global aparte (0x00559CCC).  Como nadie lo escribia, el
// gate `m_iMatchTime > 0` del renderer era siempre falso y el cartel del evento
// (tiempo + contador de monstruos) no se dibujaba nunca.
```

### clearMatchInfo (línea 811)

```cpp
// clearMatchInfo @ 0x0047EB80 (31 bytes).
// OJO: `functions.h` tenia esto mapeado a FUN_004827a0, que es una direccion
// EN MEDIO de sub_4824C0 (el scan de flechas del inventario) -- por eso el stub
// vacio.  La direccion real es 0x0047EB80.
// IDA: clearMatchInfo (0x0047EB80)
```

### getMonsterName (línea 827)

```cpp
// RE-ACTIVADO 2026-07-24: la tabla ahora esta bien dimensionada (512 × 0x36) y
// la carga NPCName_Load con Type[0]/Name[1].  IDA: `mov dl,[eax]` (Type es un
```

### FUN_00405240 (línea 846)

```cpp
// === FUN_00405240 — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
```

### FUN_00405290 (línea 883)

```cpp
// === FUN_00405290 — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
```

### FUN_0053ad80 (línea 896)

```cpp
// === FUN_0053ad80 — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
```

### FUN_0053cc00 (línea 903)

```cpp
// === FUN_0053cc00 — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
```

## `src/Core/String_ResourceLegacy.cpp`

### cabecera del archivo (línea 2)

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### declaraciones de Cloth_Integrate / Cloth_Solve (línea 10)

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Resource_Load (línea 59)

```cpp
// Pipe_SetTarget not implemented — returning 0 (no-op stub).
```

## `src/Core/System_Legacy.cpp`

### cabecera del archivo (línea 2)

```cpp
// Extracted from stubs_bulk_misc.cpp; IDA provenance comments retained.
```

### AntiTamper_HashNode (línea 33)

```cpp
// Por politica del proyecto la hash-table es anti-tamper y esta neutralizada:
// GetIndex (aca arriba) devuelve -1 siempre, asi que la rama de allocacion corre
// en CADA pasada y el puntero se pierde en el acto -- el Insert no lo guarda.
//
// (Ojo: la capacidad NO es 0.  g_HashTableCtx la deja en 1 con un slot centinela,
// asi que el early-out de HashTable_Insert no dispara y el Insert llega a correr
// su sondeo; simplemente no hace nada util con el nodo.)
//
// Eso es un LEAK, y de los grandes, porque varios de esos sitios estan en
// caminos per-frame.  El de SecondPassword_Screen4 aloca 12 nodos por frame
// (un bucle de 0x330 con paso 0x44): 12 * 1413 = ~17 KB por frame, ~1,5 GB por
// hora a 25 fps.  Un cliente de 32 bits agota su espacio de usuario en poco mas
// de una hora y operator new tira std::bad_alloc, que nadie captura -> crash
// 0xE06D7363.  Reportado 2026-09-30 con el proceso corriendo 62,5 minutos.
//
// Como la tabla esta muerta nadie LEE esos nodos -- lo unico que se hace con
// el puntero es escribirle el byte +0x584 y pasarlo al Insert, que lo tira.
// Asi que devolvemos siempre el mismo buffer: la memoria sigue siendo valida y
// escribible, y el leak desaparece.  Es completar la neutralizacion que ya
// estaba a medias, no una desviacion nueva.
//
// Los sitios escriben su flag en offsets distintos segun el tipo de nodo (+1, +4,
// +0x161, +0x584), asi que al compartir el buffer esas marcas se pisan entre si.
// Es inocuo: nadie LEE los nodos, y las dos unicas salidas de HashTable_Insert
// son escribir en un array muerto de 1 slot o llamar a CErrorReport_Write, que en
// este build es un stub vacio.  Ninguna rama es observable.
//
```
