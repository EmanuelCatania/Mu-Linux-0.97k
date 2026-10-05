# Hash-table anti-tamper: estado en runtime y bucles inline que quedan

Análisis hecho en la Fase 1 sobre `fase/1` después de #74. Sirve para decidir qué
se puede borrar de la hash-table anti-tamper sin cambiar el comportamiento del
cliente. Conclusión: **sólo se pudo probar equivalencia en `Scene_MapTick`**; el
resto de los bucles inline depende del orden de ejecución en runtime y se deja.

## 1. Estado de la tabla en este build

`g_HashTableCtx` (ex `DAT_055c9bc8`, `globals.cpp`), inicializado por `HashCtxInit_t`:

| Campo | Valor inicial |
|---|---|
| `[0]` vtable (`MAIN_HASH_CLASS`) | `g_FakeHashVtable`; la entrada `+0xC` es `HashFn_Sentinel`, que devuelve **0** |
| `[1]` values (`DAT_055c9bcc`) | `g_HashValueArr[1]`, con `values[0] = &g_HashSentinelNode` |
| `[2]` keys (`DAT_055c9bd0`) | `g_HashKeyArr[1]`, con `keys[0] = 0` |
| `[3]` capacidad (`DAT_055c9bd4`) | **1** |

Con capacidad 1 y hash 0, todo sondeo mira sólo el slot 0 y hace una sola vuelta.

Hay **tres familias de lecturas**, que se comportan distinto:

1. `HashTable_GetIndex(ctx, key)` de 2 argumentos: `inline` en `functions.h`,
   devuelve la constante `0xffffffff`. Lo que depende de él es decidible en
   compilación (es lo que borró #74 y, en este PR, `Scene_MapTick`).
2. `HashTable_GetIndex` de 3 argumentos (`Core/System_Legacy.cpp`): también
   devuelve siempre `0xFFFFFFFF`, sin efectos.
3. **Los bucles inline de Ghidra/Hex-Rays** que comparan byte a byte contra
   `keys[0]` y, si coincide, usan `values[0]`. Estos **leen el estado real** de la
   tabla.

## 2. Quién escribe `keys[0]` / `values[0]`

| Escritor | Qué escribe | Cuándo |
|---|---|---|
| `HashTable_Insert` (`Net/Crypto.cpp`) | `keys[0] = *(DWORD*)node` (los 4 primeros bytes del nodo), `values[0] = 0` (`unaff_retaddr`) | si `keys[0]` es 0 o ya vale eso; si no, sólo `CErrorReport_Write` |
| `Render/Effect_Create.cpp` (6 inserts inline) | `keys[0] = DAT_07cf1ffc` (CharacterMachine) u otro puntero, `values[0] = param_3` (un argumento de `CreateEffect`) | sólo si `keys[0] == 0` |
| `Render/Joint_Create.cpp` (2 inserts inline) | `keys[0] = values[0] = pbVar10` | sólo si `keys[0] == 0` |
| `Render/RenderLinkObject.cpp` | `keys[0] = values[0] = param_4 + 0x302` (dirección de un campo de la entidad) | sólo si `keys[0] == 0` |
| `Game/Game_MainLoop.cpp` rehash | realoca `keys`/`values` (heap, offset aleatorio) y re-inserta las claves no nulas con su valor | cada vez que `DAT_083a7c00 % 0x139 == 0x39` |

El nodo de `HashTable_Insert` es siempre `AntiTamper_HashNode()` (un buffer
estático). Sus 4 primeros bytes valen `0x00000000` hasta que algún sitio escribe
`node[1] = 1` (`CreateSocket`, `Game_CharSelectTick`, `SecondPassword`); desde
ahí valen `0x00000100`.

**Consecuencia:** `keys[0]` arranca en 0 y **el primer escritor que lo encuentra
en 0 lo fija**. Puede quedar en `0`, `0x100`, el puntero de CharacterMachine, un
`pbVar10` de `Joint_Create` o una dirección `entidad + 0x302`, según **qué función
corra primero** (pantalla de login, conexión, char select, primer efecto…). Una vez
distinto de 0, ningún insert lo vuelve a cambiar. El rehash conserva la clave.
`values[0]` vale 0 desde el primer `HashTable_Insert` o un puntero ajeno (un
argumento de `CreateEffect`, una dirección de entidad) si lo fijó un insert inline.

## 3. Bucles inline: rama que se toma y resultado

Notación: *encontrado* = el operando de búsqueda coincide con `keys[0]`; *vacío* =
`keys[0] == 0`; *lleno* = ni lo uno ni lo otro (agota la vuelta, llama a
`CErrorReport_Write`, que en este build no hace nada, y sigue por la rama de alta).

| Función | Operando de búsqueda | Rama en runtime | Qué hace la rama *encontrado* | Resultado |
|---|---|---|---|---|
| `Scene_MapTick` (`Scene/Scene_MapTick.cpp`) | — (usa `HashTable_GetIndex` inline) | siempre "no encontrado" (constante en compilación) | — | **Borrado** en este PR: `else`, `LAB_004f6614` y el decremento con `Packet_EncryptBuffer` eran inalcanzables. Las altas quedan. |
| `CreateEffect` (`Render/Effect_Create.cpp`, ~230 líneas en ≥6 casos) | según el caso, el cero de un local (`param_8`, `param_7`) o un argumento (`param_4` = SubType) | *vacío* → alta inline que **escribe `keys[0]`**; *lleno* → error + alta que no escribe; *encontrado* si `param_4 == keys[0]` | `Packet_DecryptBuffer(CharacterMachine, values[0])`: copia 0x584 bytes desde `values[0]` sobre CharacterMachine | **Dejado.** Depende de `keys[0]`, que depende del orden de ejecución; y la rama de alta escribe la tabla que leen los demás. |
| `CreateJoint` (`Render/Joint_Create.cpp`, 2 sitios) | locales `local_84` / `local_8c` | igual que arriba; la alta escribe `keys[0] = values[0] = pbVar10` | `Packet_Decrypt*` sobre `pbVar10` | **Dejado** (mismo motivo). |
| `RenderLinkObject` (`Render/RenderLinkObject.cpp`) | `pbStack_26c` (un `operator new(2)` recién pedido) y `local_23c_f` (0.0f) | *vacío* → alta que escribe `keys[0] = values[0] = param_4+0x302` | descifra `*(param_4+0x302)` desde `values[0]` | **Dejado.** Además pide `operator new(2)` en cada llamada y no lo libera (ver bugs). |
| `Weather_Update` (`Render/Weather.cpp`) | `local_40` (0) y `uStack_48` (**sin inicializar**) | *vacío* → `HashTable_Insert`; si `uStack_48 == keys[0]` → *encontrado* | toma el nodo con `HashTable_GetIndex` inline (-1) → `puVar10 = NULL` → **desreferencia NULL** | **Dejado.** La rama depende de basura de la pila. |
| `CreateSocket` (`Net/Net_Connect.cpp`, bucle de `DAT_05826ceb`) | `local_4` y otro local | *vacío* → `HashTable_Insert` | `HashTable_GetNode` + `Packet_DecryptByte` sobre `DAT_05826ceb` | **Dejado.** |
| `Scene_Intro` (`Scene/Scene_Intro.cpp`, `iStack_10` y `+0x38c`) | `iStack_10` / `uStack_4` | *vacío* → `HashTable_Insert` | `HashTable_GetNode` + `Packet_DecryptDword` | **Dejado.** Corre antes de conectar, cuando `keys[0]` suele seguir en 0, pero no se puede probar para todo arranque. |
| `AttackEffect` (`Combat/Combat_AttackEffect.cpp`, ≈60 bloques) | `Owner + 770` (dirección del byte de skill de la entidad) contra `keys[0]` | *vacío* → alta no-op (macros `AE_ht_*`); *lleno* → error + alta no-op | descifra `*(Owner+770)` desde `values[0]`; el decremento hace `*(Owner+770) = rand()` | **Dejado.** Si `RenderLinkObject` fijó `keys[0] = entidad+0x302` y esa entidad ataca, **coincide** (770 = 0x302): la rama *encontrado* corre. |
| `Terrain_Render` (`Render/Terrain_Render.cpp`) | — | `AntiTamper_HashNode` + `HashTable_Insert` | — | **Dejado** (escribe la tabla). |
| `Game_MainLoop` rehash (`Game/Game_MainLoop.cpp:96`) | — | cada 0x139 ticks, con `DAT_083a7c00 % 0x139 == 0x39` | — | **Dejado:** llama a `_rand()` tres veces; quitarlo cambia la secuencia de `rand()` del juego. |
| Altas `AntiTamper_HashNode` + `HashTable_Insert` en `Game_MainLoop`, `Game_CharSelectTick`, `Game_EnterWorldTick`, `Scene_Dispatch`, `HUD_Pass1`, `SecondPassword`, `Net_Connect`, `Scene_Intro`, `Scene_MapTick` | — | siempre (el `GetIndex` previo es constante -1) | — | **Dejadas:** escriben `keys[0]`, que leen los bucles de arriba. |

## 4. Por qué no se puede "reemplazar por la lectura en claro"

Para reemplazar un bucle por "no hacer nada" o "leer en claro" hay que probar que
nunca toma la rama *encontrado* **y** que su rama de alta no cambia lo que leen
los otros bucles. Ninguna de las dos cosas es local:

- el valor de `keys[0]` lo fija el **primer** escritor en runtime (login, conexión,
  char select, primer efecto o primer objeto enlazado), y hay al menos cinco
  candidatos con claves distintas;
- hay operandos de búsqueda que son locales sin inicializar (`Weather_Update`) o
  argumentos de la función (`CreateEffect`), así que la coincidencia depende de los
  datos;
- `AttackEffect` busca exactamente el mismo tipo de clave (`entidad + 0x302`) que
  escribe `RenderLinkObject`, así que la rama *encontrado* es alcanzable en
  principio.

Mientras quede un solo bucle inline, `HashTable_Insert`, `HashTable_GetNode`, los
`Packet_*crypt*`, `AntiTamper_HashNode`, `g_HashTableCtx`, `HashFn_Sentinel` y
`g_FakeHashVtable` siguen teniendo usuarios y **no se borran**.

## 5. Cómo seguir (Fase 2, con testing en runtime)

1. Instrumentar en Debug quién fija `keys[0]` y si alguna vez se toma una rama
   *encontrado* (un contador por sitio alcanza), y jugar las escenas que usan esos
   bucles.
2. Si nunca se toma ninguna, reemplazar **todos los bucles a la vez**: los lectores
   por su rama "no encontrado" y los escritores por nada. Entonces sí desaparecen
   `HashTable_Insert`, `HashTable_GetNode`, `Packet_*crypt*`, la tabla y el nodo
   compartido, en un solo cambio con su prueba en runtime.
3. El rehash de `Game_MainLoop` puede reemplazarse por tres `_rand()` sueltos si se
   quiere conservar la secuencia de números aleatorios.
