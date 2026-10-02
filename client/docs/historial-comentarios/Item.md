# Historial de comentarios: `src/Item/`

Comentarios de desarrollo movidos desde `src/Item/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Item/Item_ChaosMix.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Item_ChaosMix.cpp
//
// Extracted from stubs_game.cpp.  Owns chaos-mix recipe validation and its
// inventory-panel helper.  IDA provenance remains in the function comments.
```

### Línea 572

```cpp
//
// 2026-09-26: aca habia una copia bajo el nombre RenderInventoryInterface.  Las dos
// implementaciones son equivalentes; se deja una sola, con el nombre de IDA.
```

## `src/Item/Item_ClickHandler.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Item_ClickHandler.cpp
//
// Port FIEL del IDA decompile `sub_4D23B0` (`004D23B0_sub_4D23B0.c`, 14783
// bytes / ~1521 líneas). Esta es la función central de **interacción con
// items** del cliente: render del grid + hit-test + click handlers para
// pickup / drop / sell / right-click-use / hotkey-assign / pet-renaming.
//
// Llamada por SecondPassword.cpp:712-782 (4-7 sitios) per scene tick:
//   - Main inventory: `FUN_004d23b0(InventoryStartX+15, InventoryStartY+200,
//     OffsetInventoryItems, 8, 8, 0)`
//   - Trade slots (own/peer): grids 8x4
//   - Warehouse: 8x15 con flag=1 (sell mode) o flag=0 (open mode)
//   - Mix: 8x4
//
// La firma original en IDA es:
//   void __cdecl sub_4D23B0(char *a1, int a2, __int16 *a3, int a4, int a5,
//                            char a6)
// Donde:
//   a1 = origin_x  (top-left grid X en pantalla)
//   a2 = origin_y  (top-left grid Y)
//   a3 = pointer al ITEM array (Inventory / Trade / Warehouse / Mix)
//   a4 = grid width (typically 8)
//   a5 = grid height (8 / 4 / 15)
//   a6 = mode flag:
//        0 = inventory mode (left-click pickup, right-click use, hotkey)
//        1 = modo venta (el click izquierdo vende el item al NPC vía el paquete 0x32)
//
// **Anti-tamper noise omitido** per project policy:
//   - Bloques XOR con keys v263..v294 alrededor de cada packet send
//   - HashTable encrypt/decrypt sobre CharacterMachine (sub_4041E0,
//     sub_403F80, sub_404280, sub_404330, sub_404370, sub_404400, sub_423710)
//   - Repeated key init + reverse loops (compiler artifact / anti-tamper)
//
// Lo que SÍ está aquí (game logic):
//   - Hit-test mouse over cell + multi-cell footprint highlight
//   - Pickup: qmemcpy slot bytes → pPickedItem buffer + set source slot
//   - Venta a NPC: arma el paquete 0x32 y lo manda por el socket
//   - Repair: build packet 0x35 (sub-mode 1 / 2 / 3)
//   - Use potion/scroll: build packet 0x29 + EnableUse=10 cooldown
//   - Right-click "open scroll" specials: 0x49 sub 0x91 type 1/2 (item 467/434)
//   - Right-click pet egg (item 431): qmemcpy slot a word_7EA5240 + ShowCheckBox
//   - Right-click default: pickup with WarehouseOpened auto-drop logic
//   - Ctrl+Q/W/E hotkey assignment: dword_559C60[slot] = item type
//
// 2026-05-08: port completo desde IDA (sustituye al stub no-op anterior en
// SecondPassword.cpp:2027). Habilita la cadena entera:
//   FUN_004d23b0 → pPickedItem set → Inventory_DropDispatch (drop dispatcher,
//   también stub — port pendiente) → SendRequestEquipmentItem.
```

### Línea 80 — antes de `#define dword_5826D18      BuyCost`

```cpp
// 2026-08-22 FIX: este alias apuntaba a DAT_05826d1c, que es OTRO global.
// `ida_xrefs_to` los separa: 0x05826D18 lo escribe ProtocolCore y lo lee
// sub_4D23B0 (cooldown de COMPRA), mientras 0x05826D1C lo escriben InitGame,
// ReceiveLife y ReceiveDurability (cooldown de equipar/usar, `EnableUse`).
// Compartiendo el mismo byte, comprar bloqueaba el equipar y viceversa.
// Las dos direcciones están a ~4 bytes; acá las tratamos como el mismo concepto.
```

### Línea 88 — antes de `extern DWORD DAT_07eaa160;                     // IDA 'CheckInventory'`

```cpp
// CheckInventory: aliasa el puntero al ITEM del slot bajo el mouse que usa Scene_MapTick
// to dispatch RenderItemInfo (tooltip).
//
// 2026-05-08: BUG-FIX MAYÚSCULO. Antes apuntaba a `DAT_07e11d24` que IDA
// llama `dword_7E11D24` (= un global completamente distinto, usado por
// sub_494520 IME/text input). El símbolo correcto es `DAT_07eaa160` —
// confirmado por Ghidra-decompiled Scene_MapTick línea 35/89 que lee
// `DAT_07eaa160` como el item pointer y línea 89 lo pasa como 3er arg
// a `RenderItemInfo` (RenderItemInfo).
//
// Sin este fix:
//   * Hover loop seteaba DAT_07e11d24 (wrong global) → Scene_MapTick leía
//     DAT_07eaa160 (= 0) → no entraba a la rama de tooltip → nunca se
//     renderizaba RenderItemInfo.
//   * Y peor: estábamos contaminando dword_7E11D24 que es input-buffer-state
//     → bugs latentes en chat input.
```

### Línea 188 en `ItemMove_SnapMouseToEmptySlot` — antes de `if (*(short*)cell == (short)0xFFFF) {`

```cpp
                    // IDA sub_4D6020: libre = Type == 0xFFFF, nada mas.  Key
                    // vale 0 en las celdas NO primarias de un item multi-celda,
                    // asi que el `|| Key <= 0` que habia aca daba por libres
                    // celdas ocupadas y el quick-move soltaba encima de otro
                    // item (ver [[celda-ocupada-se-decide-por-type]]).
```

### Línea 221 — antes de `extern char DAT_083a44c4[7 * 0x26];      // g_lpszMessageBoxCustom (266 bytes)`

```cpp
//
// 2026-05-08: port completo, reemplaza al placeholder que llamaba a
// CreateOkMessageBox. Maneja la máquina de estados del diálogo en la que
// Inventory_DropDispatch se apoya para los flujos de confirmación de venta/drop/renombrar mascota.
```

### Línea 246 — antes de `extern DWORD DAT_083a7c24;               // ErrorMessage (currently-shown dialog)`

```cpp
// NextErrorMessage vive en DAT_083a7c28, según las notas de la máquina de estados de CLAUDE.md.
```

### Línea 299 en `ShowCheckBox` — antes de `extern DWORD DAT_083a4324;`

```cpp
    // 2026-07-27 FIX (cartel de confirmación vacío): el render de las líneas del
    // message box (UI_StatsPanel case 0x97/0x99) usa DAT_083a4324 como count del
    // loop, pero ShowCheckBox sólo seteaba g_iNumLineMessageBoxCustom. En el
    // binario original son la MISMA dirección; en nuestro build están separados
    // → el render iteraba 0 líneas → cartel en blanco. Seteamos ambos.
```

### Línea 372 — antes de `extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);`

```cpp
// Build & send a C1-header packet [C1][size][header...payload].
//
// 2026-08-08 FIX MAYÚSCULO (desconexiones "de la nada" + al subir stats):
// esto incrementaba `DAT_05826ceb` (g_byPacketSerialSend) en CADA envío C1.
// Pero el serial SÓLO viaja en los frames C3/C4: el server lee
// `QueueInfo.serial = DecSerial` para C3/C4 y `serial = -1` para C1/C2
// (SocketManagerLinux.cpp:248-316), y sólo entonces avanza su `m_RecvSerial`
// (CSerialCheck::CheckSerial exige `m_RecvSerial + 1 == serial`).
// O sea: cada packet C1 que salía por acá corría NUESTRO contador sin que el
// server corriera el suyo → el siguiente packet C3 llegaba con el serial
// adelantado → `CheckPacketHack` loguea "Packet serial error" y hace
// CloseClient. Como el keep-alive 0x0E es C3 y sale cada segundo, la
// desconexión llegaba ~1 s después de cualquier click que mandara un C1
// (de ahí el "me desconectó estando quieto" tras usar el diálogo de venta).
// El bump lo hace Net_SendSmallPacket, que es quien realmente escribe el
// serial en el frame.
```

### Línea 390 — antes de `static void SendC1Packet(BYTE* payload, int payloadSize)`

```cpp
// SendC1Packet — envuelve un payload en frame C1 y lo manda.
//
// 2026-08-26: antes construia el frame a mano y lo pasaba a SendPacketBytes,
// que llama a ::send directo — o sea SIN el chain-XOR. Pero el server aplica
// `XorData` a TODO frame C1 (`ExtractPacket` -> `XorData(size-1, 2)`), asi que
// des-XOR-eaba un paquete que nunca fue XOR-eado y veia basura de pkt[3] en
// adelante. Solo se salvaban los de 3 bytes, donde el bucle del server no
// itera. Los dos call sites que quedan (scrolls 467/434, opcode 0x49/0x91, 7
// bytes) caian de lleno en el bug.
//
// Ahora delega en Net_SendC1Packet, que aplica el chain-XOR y ademas resuelve
// el frame contra la tabla de HackPacketCheck.
```

### Línea 413 — antes de `void Net_SendSmallPacket(const BYTE* pkt, int totalLen);`

```cpp
// 2026-07-27: envío C3 (CSimpleModulus + serial) para los opcodes de tienda
// que el server exige con Encrypt=1 (HackPacketCheck.txt): 0x32 buy, 0x33 sell,
// 0x23 drop. Enviarlos como C1 (SendC1Packet) → el server los rechaza en
// HackPacketCheck ("Packet encryption error") → CloseClient (desconexión al
// clickear un item de la tienda). Net_SendSmallPacket arma [C1][len][head]...,
// pisa len con el serial, aplica chain-XOR + CSM y emite el frame C3 final.
```

### Línea 430 — antes de `static bool GetHeroDropTile(BYTE* outX, BYTE* outY)`

```cpp
// 2026-07-27: tile del terreno donde dropear un item al suelo. El server
// (CGItemDropRecv) usa x/y como TILE del mapa y valida cercanía al player —
// mandar pixels de mouse (lo que hacía el port) daba un tile off-map →
// gMap.ItemDrop rechazaba → result=0 → el item nunca se dropeaba. Usamos el
// tile actual del héroe (ent+0x306/0x307 = target_grid_x/y), siempre válido y
// pegado al player.
// 2026-09-09: devolvia `hero[0x306]/[0x307]`, que NO es la posicion del heroe
// sino su GRILLA DESTINO (a donde esta caminando).  El item caia en cualquier
// lado -- ni donde estaba el jugador ni donde soltaba el mouse.
//
// IDA (sub_4DF410) manda `(int)(xf * 0.0099999998)` y `(int)(yf * ...)`, donde
// xf/yf son CollisionPosition, el punto del terreno bajo el CURSOR:
//     DAT_083a4130 / DAT_083a4134, los mismos que ya usa el click-to-move
//     (Combat_Targeting.cpp) con la formula identica `* 0.01f`.
// MU 5.2 lo confirma (NewUIMyInventory.cpp L512-515):
//     RenderTerrain(true);
//     if (RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, ...))
//         SendRequestDropItem(slot, (int)(CollisionPosition[0] / TERRAIN_SCALE),
//                                   (int)(CollisionPosition[1] / TERRAIN_SCALE));
// o sea el pick de terreno se valida ANTES de mandar.
// Devuelve false si el cursor no esta sobre terreno: IDA hace `return` en ese
// caso (sub_4DF410 L1070-1073) y el item queda en la mano.  Antes caia a la
// celda del heroe, que no es lo que hace el original.
```

### Línea 471 — antes de `void RestorePickedItemToSource(void)`

```cpp
// 2026-07-27: devuelve el item agarrado a su slot de origen y suelta el cursor.
// Se usa al cancelar los diálogos de confirmación (venta / drop al suelo); sin
// esto el item quedaba pegado al mouse y no había forma de soltarlo.
// No es `static`: la usa tambien el handler del 0x33 (venta rechazada por el
// server) en Net/Net_Process.cpp.
```

### Línea 632 en `RestorePickedItemToSource` — antes de `memset(wearSlot, 0, sizeof(ITEM));`

```cpp
        // 2026-08-08 FIX (glow dorado pegado al desequiparse):
        // este clear era PARCIAL — ponía Type=-1, Key=-1 y el byte +27, pero
        // dejaba el campo **Level** (+4) con el valor del item que se acababa de
        // sacar. El diagnóstico SETGLOW lo mostró:
        //   cmT=242/-1/-1/338/370   cmL=11/11/11/11/11
        // o sea los slots vaciados seguían con nivel 11. Después
        // `SetCharacterClass` copia ese nivel al body-part por defecto
        //   *(BYTE*)(c+530) = (*(int*)(CM_armor + 4) >> 3) & 0xF
        // → lvlE=11 sobre el cuerpo desnudo (partsE=923/930 = modelos default)
        // → `Entity_DrawSetup` le aplica el glow de +11 al cuerpo desnudo.
        // Por eso el pj quedaba dorado aunque `CheckFullSet` ya devolvía 0.
        // Dejamos el slot en el MISMO estado que produce
        // `WriteEquipmentSlot(slot, -1, …)` (memset + Type=-1), que es también
        // el que manda el server en el snapshot F3/10.
```

### Línea 692 en `FUN_004d23b0` — antes de `if ((int)EnableUse > 0)         { return; }`

```cpp
    // (2026-09-11: aca habia una pre-pasada que ponia Color = 0 en todo el
    //  pool.  El reset lo hace sub_4E6550 una vez por frame, antes de esta
    //  funcion y de sub_4DF410 — ver el port en SecondPassword.cpp.  Esta
    //  funcion tambien se llama desde el RENDER, despues del marcado del drop,
    //  y la pre-pasada borraba esas marcas: por eso no se veia la silueta.)
```

### Línea 699 en `FUN_004d23b0` — antes de `if (DAT_07eaa165 != 0) {`

```cpp
    // 2026-07-27 FIX (baúl: no se puede meter ni sacar nada): DAT_07eaa165 es el
    // guard "item-move en vuelo" — se setea al mandar el 0x24 y sólo lo limpia
    // la RESPUESTA del server (ItemMove_ClearPickedState). Si un move se pierde
    // o el server no responde, el guard queda pegado en 1 y ESTE early-return
    // bloquea TODO el manejo de inventario/baúl para siempre. Timeout de
    // seguridad: si lleva >2 s seteado, lo liberamos.
```

### Línea 748 en `FUN_004d23b0` — antes de `ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;`

```cpp
            // ── Hovered cell with item: highlight footprint (color=2) ──────
            // 2026-05-08: defensive guards. Crash reported with addr=0x21
            // (= offset de ITEM_ATTRIBUTE.Height) al pasar sobre un item, que
            // means `attr + type*64` reduced to NULL. Possible causes:
            //   * DAT_07d78068 not yet initialized (loader race)
            //   * `type` fuera de rango (rowSlot apuntando a basura)
            // Los dos están acotados ahora.
```

### Línea 820 en `FUN_004d23b0` — antes de `extern DWORD DAT_07ea840c;   // tooltip X (= IDA 'sx')`

```cpp
            // ── 2026-05-08: escribe los globals de posición del tooltip (sx/sy en IDA) ──
            // Scene_MapTick los lee en cada frame y se los pasa a
            // RenderItemInfo. Sin estas escrituras el tooltip no aparece nunca
            // (o aparece en 0,0). Per IDA L388-395:
            //   sx = origin_x + 20*slotX + 20*ItemAttribute[type].Width / 2
            //   sy = origin_y + 20*slotY
```

### Línea 845 en `FUN_004d23b0` — antes de `if (mode_flag != 0) {`

```cpp
            // ── BRANCH B: mode_flag != 0 → SHOP BUY (click en item de tienda) ──
            // 2026-07-27 FIX: esta rama sólo la alcanza el shop (único caller con
            // mode_flag=1, SecondPassword.cpp:818). NO es sell — es BUY: clickeás
            // un item del grid de la tienda para comprarlo. El port anterior:
            //   (1) lo etiquetó "sell" y mandó `0x32 0x01 slot 0` (sub-byte que el
            //       server 0.97k/MuEmu NO usa → leía slot=0x01), y
            //   (2) lo envió como C1, pero PMSG_ITEM_BUY_RECV (0x32) exige C3
            //       (HackPacketCheck Encrypt=1) → CloseClient = la desconexión que
            //       el usuario veía al tocar un item de la tienda.
            // Server espera: [C1][04][32][slot], C3-encrypted (ItemManager.h:63).
```

### Línea 873 en `FUN_004d23b0` — antes de `if (DAT_07eaa134 != 0) {`

```cpp
            // ── BRANCH C: RepairEnable mode (NPC repair UI active) ─────────
            // 2026-05-09 FIX: per IDA xrefs `RepairEnable_0` = address
            // 0x07EAA134 (= our DAT_07eaa134, the B-key/repair-mode flag).
            // Antes usábamos `DAT_07e11d18` que es OTRO global no relacionado;
            // pero como siempre vale 0, no era el bug. Lo dejamos al lado por
            // si un day el reading DWORD del IDA picks up DAT_07e11d18+...
            // bytes adyacentes. Lo importante: *ambos* deben ser 0 para que
            // pickup dispare. Si DAT_07eaa134 está pegado en 1 (porque
            // Scene_MapTick lo mantiene en 1 cuando DAT_07eaa138 != 0), nunca
            // hay pickup. Usar el OR para detectar el bug.
            // IDA: RepairEnable_0 (0x07EAA134).  `DAT_07e11d18` era un global
            // sin xrefs en IDA; se quito el 2026-09-11.
```

### Línea 897 en `FUN_004d23b0` — antes de `int slotIdx = ((BYTE*)rowSlot)[62] + grid_w * ((BYTE*)rowSlot)[63] + 12;`

```cpp
                    // 2026-08-08 FIX: el port mandaba `[0x35][0x01][slot][flag]`
                    // — opcode inventado. El server MuEmu no tiene case 0x35
                    // (Protocol.cpp) así que el paquete se descartaba y reparar
                    // nunca hacía nada. Per IDA sub_4D23B0 L588-640 el paquete es
                    //   [C1][05][34][slot][RepairEnable]
                    // = PMSG_ITEM_REPAIR_RECV (ItemManager.h:75), con
                    // RepairEnable = 0 (reparar en NPC) / 1 (auto-reparar).
```

### Línea 994 en `FUN_004d23b0` — antes de `if (type == 467 || type == 434) {`

```cpp
                // ── Items 467 / 434 — consulta de tiempo del evento ────────
                // Click derecho sobre "Devil's Invitation" (467) o "Cloak of
                // Invisibility" (434): NO entra al evento, pide cuanto falta
                // para que abra. Entrar es aparte, desde el NPC del evento
                // (0x90 Devil Square / 0x9A Blood Castle).
                //
                // Server: PMSG_EVENT_REMAIN_TIME_RECV (Protocol.h:63)
                //     [C1][05][91][EventType][ItemLevel]
                // EventType 1 = Devil Square, 2 = Blood Castle; el server hace
                // `ItemLevel - 1` para indexar el nivel del evento.
                // La respuesta (mismo opcode 0x91) la atiende
                // `Recv_EventZoneOpenTime` en src/Net/Net_Events.cpp.
                //
                // 2026-08-26: el port anterior mandaba
                //     [C1][07][49][91][subtype][slot][level]
                // o sea con un opcode 0x49 inexistente, un byte de mas y el
                // slot que el server no espera; el server lo ignoraba en
                // silencio y por eso el click derecho no hacia nada. El 0x49
                // salio de leer mal el decompile: en `sub_4D23B0` (raw
                // L1032-1053) `v275 = 73` es el byte 12 de la CLAVE XOR (0x49),
                // no un opcode — aparece 10 veces en la funcion porque la clave
                // se re-arma antes de cada envio. El opcode real es
                // `v301[4] = -111` = 0x91, y el EventType es `v302` (1 para el
                // 467, 2 para el 434).
```

### Línea 1143 en `FUN_004d23b0` — antes de `if (DAT_07eaa119 != 0) {`

```cpp
                    // Pickup from warehouse / mix / trade
                    // 2026-07-27 FIX (no se podían sacar items del baúl): se
                    // pasaba `&InventoryStartX` (la DIRECCIÓN del global) como
                    // origen X del grid destino en vez de su VALOR → el scan de
                    // slot libre en el inventario devolvía 0 → byte_83A42EB=0 →
                    // el pickup nunca se completaba. El origen del grid del
                    // inventario es (InventoryStartX+15, InventoryStartY+200),
                    // igual que en el resto de los call sites.
```

### Línea 1207 — antes de `// Inventory_DropItemEx: real entry point with explicit screen origin/grid`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// Inventory_DropDispatch — port FIEL desde IDA `004DF410_sub_4DF410.c` (8067 bytes).
//
// Dispatcher de drop del inventario: punto de entrada por frame que llama el tick
// PacketUpdate de la escena (Net_PacketSession.cpp:284). Cuando el jugador tiene
// un item levantado (dword_7E91388 > 0), esta función llama a
// `Inventory_DropItem` (FUN_004D6470 = sub_4D6470) up to four times — once
// por cada contexto de inventario visible: principal, trade, baúl y mix. Cada
// llamada intenta soltar el item si el mouse-up cae sobre esa grilla.
//
// If no drop succeeded AND ShopOpened is set: special bless/luck/level
// guards antes de mandar el paquete de venta al NPC. Si no: soltar al piso
// (item-drop packet 0x23).
//
// State machine: dword_7EAA13C tracks "sell-confirm" / "drop-confirm"
// dialogs spawned via ShowCheckBox(3, ...) — case 1 = sell ok, case 2 =
// drop ok. Both are completed via byte_559F5E response (1=yes, 2=no).
//
// Anti-tamper: cada envío de paquete pasa por la misma encriptación XOR /
// hash-table noise pattern documented in Item_ClickHandler.cpp's main
// dispatcher. Skipped per project policy.
//
// 2026-05-08: port completo. Reemplaza al stub no-op anterior en
// Net/SecondPassword.cpp:153 que decía "STUB: SEH + HashTable".
// ─────────────────────────────────────────────────────────────────────────────
```

### Línea 1273 en `Inventory_DropDispatch` — antes de `DAT_07eaa13c = 0; DAT_00559f5e = 0;`

```cpp
            // User confirmed sell-to-NPC. Server: [C1][04][33][slot], C3
            // (PMSG_ITEM_SELL_RECV, Encrypt=1). Antes: sub-byte 0x01 + C1 →
            // desconexión.
```

### Línea 1282 en `Inventory_DropDispatch` — antes de `DAT_07eaa13c = 0; DAT_00559f5e = 0;`

```cpp
            // 2026-07-27 FIX (item pegado al mouse): al CANCELAR el confirm se
            // limpiaban los flags del diálogo pero el item quedaba "en la mano"
            // (dword_7E91388=1) y sin volver a su slot → arrastrado por el cursor
            // para siempre, y cualquier movimiento re-disparaba el cartel.
            // Ahora lo devolvemos al slot de origen y soltamos el cursor.
```

### Línea 1295 en `Inventory_DropDispatch` — antes de `DAT_07eaa140 = 1;`

```cpp
            // 2026-09-11: se quito una rama que, con una receta no
            // reconocida, TIRABA AL SUELO el item de la mano (0x23): el
            // original no tiene confirmacion de drop al suelo.
```

### Línea 1358 en `Inventory_DropDispatch` — antes de `// Grilla de mezcla del caos (8x4) si está abierta y la mezcla no se está procesando`

```cpp
    // (diag WHDROP removido 2026-08-08 — ya cumplió: el drop sobre el baúl
    //  funciona y el bug de mover DENTRO del baúl era el clobber de
    //  dword_7EA9800 en Inventory_DropItemEx, no la conversión mouse→celda.)
```

### Línea 1402 en `Inventory_DropDispatch` — antes de `if (!dropMain && !dropTrade && !dropWH && !dropMix &&`

```cpp
        // 2026-08-08 FIX ("No tienes permitido tirar este item costoso" al
        // mover un item EQUIPADO): soltar sobre una casilla de equipo caía en
        // la rama de tirar-al-suelo.
        //
        // CORRECCION 2026-09-13: `sub_4D6470` NO maneja las casillas de equipo
        // (su raw sólo llama a sub_4D5D70 y sub_4CD3B0; cubre las 4 grillas).
        // Devuelve 0 apenas la celda calculada da negativa — que es justo lo
        // que pasa arriba del grid (mouseY < InventoryStartY+200), o sea toda
        // la zona de equipo. En el binario ese click lo atiende el hit-test de
        // equipo del render antes de este dispatcher.
        // Esa región la maneja sub_4CDC70 (FUN_004cdc70, desde sub_4E6550),
        // así que acá salimos SIN consumir el click para que le
        // llegue. Sin esto: mensaje rojo + `RestorePickedItemToSource`, y el
        // item nunca se equipaba/desequipaba.
```

### Línea 1441 en `Inventory_DropDispatch` — antes de `const BYTE sellExcByte = *((BYTE*)pPickedItem + 0x6b - 0x44);`

```cpp
                // Items que piden confirmacion al venderlos (joyas, armas
                // especiales, +5 o mas, y EXCELLENT).  Lista 1:1 con IDA
                // sub_4DF410 L753-766.
                //
                // 2026-09-27: faltaba el ultimo termino, el de excellent.  Sin
                // el, un Excellent solo disparaba el cartel si ademas caia en
                // otro termino de la lista -- en la practica el de
                // `pickLevel > 4 && pickType < 384`.  Los anillos y pendants
                // son tipo >= 384, asi que para ELLOS no habia ningun termino
                // que matchear y se vendian directo, sin aviso.  Reportado como
                // "no sale el mensaje al vender Rings/Pendants Excellent".
                //
                // El byte de excellent es byte_7E9136B, los 6 bits bajos.  Se
                // lee con la misma expresion que la rama de tirar-al-piso de
                // mas abajo, que si lo tenia.
```

### Línea 1489 en `Inventory_DropDispatch` — antes de `{`

```cpp
            // 2026-07-27 FIX: tirar al PISO un item valioso NO abre un Yes/No
            // (ese cartel es de la TIENDA, para confirmar una venta). Per IDA
            // sub_4DF410 L950-964 el original sólo muestra un mensaje rojo que
            // lo prohíbe y NO suelta el item:
            //     UIChatLogWindow_AddText(byte_7EAA194, GlobalText[269], 2)
            // Nuestro port abría ShowCheckBox(2,...) → cartel de venta al soltar
            // en el suelo + item pegado al cursor esperando una respuesta.
            // Lista de tipos 1:1 con IDA (incluye 435 y el gate de excellent).
```

### Línea 1518 en `Inventory_DropDispatch` — antes de `if (InventoryOpened != 0 && (int)DAT_083a427c >= (int)InventoryStartX) {`

```cpp
            // 2026-08-08: guard `v144` de IDA (sub_4DF410 L966-969) que faltaba
            //   v144 = 1; if (InventoryOpened && MouseX >= InventoryStartX) v144 = 0;
            // Con el inventario abierto, un click sobre su panel NUNCA tira el
            // item al suelo (la zona de abajo del grid son la barra de zen y los
            // botones).
            //
            // 2026-09-11: el item NO queda agarrado.  En IDA, con v144 = 0 el
            // bloque del suelo no corre y la ejecucion cae en LABEL_301:
            // `sub_4CD3B0(1, 0)`, que devuelve el item a su celda.  (Confirmado
            // contra el cliente original: soltar un item sobre otro lo devuelve.)
```

### Línea 1533 en `Inventory_DropDispatch` — antes de `if ((DAT_07eaa119 != 0 || DAT_07eaa11a != 0)) {`

```cpp
            // 2026-09-09: el guard de arriba solo cubre el panel del INVENTARIO.
            // Soltar sobre el panel del BAUL o el de la CHAOS MACHINE (que van a
            // la izquierda, en dword_7EAA0C8) pero fuera de sus celdas caia al
            // fallback de "tirar al suelo": de ahi salia "No tienes permitido
            // tirar este item costoso" al querer guardar un item Excellent.
            //
            // En IDA no hace falta porque el drop sobre esos paneles lo consume
            // `sub_4D6470` entero (36 KB, maneja los cuatro grids Y sus zonas
            // muertas).  Nuestro port partio esa responsabilidad entre
            // `Inventory_DropItemEx` (solo las celdas) y este dispatcher, asi
            // que el hueco hay que taparlo aca -- mismo criterio y misma
            // desviacion que el guard de las casillas de equipo (2026-09-04).
```

## `src/Item/Item_Display.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 41 — antes de `void __cdecl RenderItemName(int i, DWORD o, int ItemLevel, int ItemOption, bool Sort) {`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// RenderItemName @ 0x004C9E70 — nombre flotante de un item del suelo
// ─────────────────────────────────────────────────────────────────────────────
// Port 1:1 del decompile (mu97k-src-IDA/raw/004C9E70_RenderItemName.c).  Lo
// llama sub_4CB6F0 (Target_Render): para el item bajo el cursor con Sort=0 y
// para todos los del suelo con Sort=1 mientras Alt esté activo.
//
//   o          = &Items[i][72]  → o+2 = índice de modelo (tipo + 400)
//   ItemLevel  = Items+8        → para el Zen (modelo 863) es la CANTIDAD
//   ItemOption = Items+31
//
// 2026-08-21: antes era un resumen escrito a ojo.  Divergencias que tenía y que
// este port corrige:
//   · Zen (863): hacía `sprintf(buf, DAT_0055a608, name)` con DAT_0055a608 = ""
//     → cadena vacía, o sea el Zen del suelo no mostraba NADA.  IDA es
//     `sprintf(String, "%s %d", name, ItemLevel)` = "Zen <cantidad>".
//   · Los colores de nivel 3-4 y de la rama (v5 & 0x87) estaban invertidos en
//     RGB (IDA llena v38[2],v38[1],v38[0] y llama glColor3f(v38[0],v38[1],v38[2])).
//   · Faltaban por completo las ramas 860 (Event), 831 (alas), 951-958
//     (flechas/bolts), 795 (pergamino de skill), 826 (piedra de invocación) y
//     los sufijos Excellent/Luck/Skill (GlobalText[176..179]).
//
// Ruido anti-tamper omitido por policy: las ramas 795 y 826 del binario están
// envueltas en lookups de hash-table (ref-count + XOR sobre la entrada de
// SkillAttribute).  Con nuestra tabla en claro el resultado es el mismo.
//
// Desviación: donde IDA hace `sprintf(String, GlobalText[N])` (una cadena de
// datos usada como formato) nosotros usamos `sprintf(String, "%s", GlobalText[N])`.
```

### Línea 346 en `Inventory_DropItemEx` — antes de `BYTE* sourceInvBase = (BYTE*)(uintptr_t)DAT_07ea9800;`

```cpp
    // Use the args directly (FIX 2026-05-08).
```

### Línea 353 en `Inventory_DropItemEx` — antes de `// ── Mouse-to-grid conversion (per IDA L595-597) ─────────────────────────`

```cpp
    // 2026-08-08 FIX "mover items DENTRO del baul los hacia desaparecer":
    // aca habia un `DAT_07ea9800 = invBase` ("update para downstream readers")
    // que es una INVENCION del port — IDA sub_4D6470 SOLO LEE dword_7EA9800,
    // nunca lo escribe (los unicos writers son sub_4D23B0 L798/L1401 y
    // Player_InputTick L711, todos en el PICKUP). dword_7EA9800 es el pool de
    // ORIGEN del item agarrado, y el dispatcher sub_4DF410 llama a esta funcion
    // hasta 4 veces por frame (main inv, trade, baul, mix). La primera llamada
    // (main inv) pisaba el origen con OffsetInventoryItems, asi que en la
    // llamada del baul `sourceMoveFlag` salia 0 (=inventario) en vez de 2
    // (=baul) -> el server recibia SourceFlag=0 con SourceSlot=101 (fuera del
    // rango de inventario) -> INVENTORY_RANGE falla -> result=0xFF y el item
    // quedaba solo borrado localmente = "desaparecio".
    // Sintoma cruzado en el log: baul->inventario (resuelto en la 1er llamada,
    // antes del clobber) SI mandaba srcF=2 y funcionaba.
```

### Línea 399 en `Inventory_DropItemEx` — antes de `unsigned long long result = CheckInventorySpace(`

```cpp
        // Call CheckInventorySpace to validate placement.
        // 2026-05-09 BUG-FIX: ANTES pasábamos `mouseGridX, mouseGridY` (= grid
        // coords ya calculadas como 0..7) como p1, p2. Pero la función espera
        // SCREEN OFFSETS (origin_x, origin_y) para hacer la conversión interna
        // mouseX-p1 → relative pixel → grid. Pasar grid coords daba
        // gridX = (MouseX - 1)*0.05 ≈ 30 → fuera del grid → emptyCount=0 →
        // spaceFree=0 SIEMPRE. Esto es por qué el drop nunca encontraba slots
        // libres aún con el watchdog de attr.
```

### Línea 424 en `Inventory_DropItemEx` — antes de `if (spaceFree) {`

```cpp
                // IDA sub_4D6470 L630-642.  El port tenia las ramas cruzadas:
                //   entra                     -> 2 (azul, InventoryColor)
                //   no entra, jewel 461/462/464 -> 4 (verde: se aplica al item)
                //   no entra                  -> 3 (rojo)
```

### Línea 490 en `Inventory_DropItemEx` — antes de `if (DAT_07eaa119 != '\0' || DAT_07eaa11b != '\0') {`

```cpp
                // Con el baul o el trade abiertos no se puede aplicar la
                // jewel: IDA salta a LABEL_807, que muestra el mensaje. Por eso
                // `canStack` queda en false en esos dos casos (antes se ponia
                // en true al final incondicionalmente y el aviso no salia).
```

### Línea 501 en `Inventory_DropItemEx` — antes de `if ((int)EnableUse < 1) {`

```cpp
                    // 2026-08-24 FIX (issue #15, "las jewels no se consumen"):
                    // aca se mandaba `SendRequestEquipmentItem`, o sea
                    // 0x24 PMSG_ITEM_MOVE_RECV (11 bytes). El server trata eso
                    // como MOVER la jewel a una celda ocupada -> lo rechaza y
                    // el cliente la devuelve al inventario. IDA (sub_4D6470
                    // L5919-5931) manda 0x26 PMSG_ITEM_USE_RECV, que es el que
                    // dispara CharacterUseJewelOfBles/Soul/Life en el server
                    // (ItemManager.cpp:2753+) y contesta con GCItemDeleteSend +
                    // GCItemModifySend (F3/14).
                    //
                    //   struct PMSG_ITEM_USE_RECV {   // ItemManager.h:49
                    //       PBMSG_HEAD header;        // C1 : 5 : 0x26
                    //       BYTE SourceSlot;          // +3
                    //       BYTE TargetSlot;          // +4
                    //   };
                    //
                    // Va por Net_SendSmallPacket porque HackPacketCheck.txt da
                    // Encrypt=1 para el indice 38 -> el frame final tiene que
                    // ser C3 con serial. El 0xC1 que arma IDA es el texto plano
                    // previo al encriptador, no el frame que viaja.
```

### Línea 522 en `Inventory_DropItemEx` — antes de `EnableUse = 10;`

```cpp
                        // IDA: `if (EnableUse > 0) goto LABEL_808;` — durante el
                        // cooldown NO se manda nada. Antes se mandaba igual.
```

## `src/Item/Item_Durability.cpp`

### Línea 1 — antes de `// stubs_helpers.cpp`

```cpp
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

### Línea 48 — antes de `unsigned int __cdecl Item_CalculateMaxDurability(void* item, int attrBase, int Level)`

```cpp
// BUG-FIX 2026-05-01: stub anterior leía *(uint*)(attrBase+0x8) que cae en
// Name[8..11] del struct ITEM_ATTRIBUTE. Para "Light Saber", Name[8..11] = "ber"
// = 0x00726562 = 25954 — exact valor visto en RenderBrokenItem ("0/25954").
// Corregido a leer p->Durability (offset +41 = +0x29).
```

### Línea 62 en `Item_CalculateMaxDurability` — antes de `if ((uintptr_t)attrBase < 0x100000 || (uintptr_t)attrBase >= 0x80000000)`

```cpp
    // 2026-05-08: bug-fix — antes solo chequeaba `attrBase == 0`, pero callers
    // pasan `Type * 0x40 + DAT_07d78068` y si DAT_07d78068 == 0 entonces
    // attrBase = Type*0x40 (un valor pequeño tipo 0x2A00 para Type=168).
    // Eso pasa el `!= 0` check pero defereferenciar p->MagicDurability (offset
    // 42) crashea con AV en addr 0x2A2A. Validamos que attrBase sea un puntero
    // razonable de heap (>= 0x100000) y que `item` también sea válido.
```

### Línea 101 — antes de `int __cdecl ItemValue_MuEmu(void* item, int goldType);`

```cpp
// IDA: FUN_0047C690
//
// IDA-ported 2026-04-26 (audit #3): el stub anterior devolvía "número de dígitos"
// en vez del valor real. Calcula precio gold del item considerando type/level/
// durabilidad/options. Si sellMode!=0 aplica reducción ×1/3 y penalty por durab.
//
// Caso especial #135 = Wings stage 1 / #143 = Wings stage 2 (jewel pricing).
// Cases 461/462/464/470/430/431/419/432-434/465-467/469/457/468 = jewels y sets.
// Constantes derivadas del binario original (`&unk_xxxxxx` en IDA = direcciones
// usadas como valores enteros — comprobado: 0x895440 = 9_000_000, etc.).
```

### Línea 123 en `ItemValue_Vanilla` — antes de `if (!item_v || (uintptr_t)item_v < 0x100000) return 0;`

```cpp
    // 2026-05-08: defensive — same problem as CalcMaxDurability/RenderItemInfo:
    // si DAT_07d78068 está en 0 (table base no inicializada), el cómputo
    // `(int)DAT_07d78068 + type * 0x40` da un valor pequeño y crashea al
    // dereferenciar p->Money / p->Level. Bail con 0 en ese caso.
```

### Línea 407 — antes de `unsigned int __cdecl Item_CalculateRepairCost(int Gold, int Durability, int MaxDurability,`

```cpp
// IDA: FUN_004C3EF0
//
// IDA-ported 2026-04-26 (audit #3): el stub anterior solo escribía "%u / %u"
// pero el real calcula gold de reparación: sqrt(sqrt(Gold)) * sqrt(Gold) * 3 *
// (1 - dur/maxDur) + 1, con bonus 1.4× si rota, +5% si RepairEnable, redondeo
// a múltiplos de 100/10, y formato "1,234,567" en Text. Devuelve gold final.
```

### Línea 492 en `Item_RecalculateRepairCost` — antes de `if ((itemType < 416 || itemType > 419) &&`

```cpp
            // Skip ring/wingtype/etc equipment IDs that don't degrade
            // BUG-FIX 2026-04-26 (audit #3): IDA real:
            //   gold = ItemValue(item, 2);
            //   DAT_07eaa0f8 += ConvertRepairGold(gold, dur, maxDur, type, buf);
            // El stub anterior pasaba (curDur, type, item, 2) → desordenado.
```

### Línea 512 en `Item_RecalculateRepairCost` — antes de `short *psVar12 = (short *)OffsetInventoryItems;`

```cpp
    // IDA sub_4C4080: segundo bucle sobre el grid del inventario,
    // OffsetInventoryItems .. 0x7EA9510 = 64 celdas de 0x44 (8x8), filtrando
    // por Key.  2026-09-12: el port recorria 8 "items" desde &DAT_07ea8410, que
    // en este build es un DWORD suelto: leia los globals vecinos (entre ellos
    // DAT_07ea840c/8408, las coordenadas del tooltip) y el costo de "reparar
    // todo" cambiaba segun el item bajo el mouse.
```

## `src/Item/Item_EquipmentAutoSwap.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 43 en `Item_AutoEquipAmmo` — antes de `const short bowSlotType      = *(const short*)(characterMachine + 604);  // IDA: v18`

```cpp
    // y CreateArrow (0x474370 L67), el arco vive en CharacterMachine + 604 y la
    // ballesta en + 536; la municion va al slot que queda libre.  El port leia
    // el arco en +536 y la ballesta en +604, o sea al reves: con arco equipado
    // ninguna rama daba, y al quedarse sin flechas (+536 == -1) caia en la rama
    // de ballesta y pedia equipar en el slot 1 — el del propio arco.
```

## `src/Item/Item_EquipmentQueries.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 15 en `Item_FindElfWeaponInventorySlot` — antes de `const char* const cmBytes = (const char*)(uintptr_t)DAT_07cf1ffc;`

```cpp
    // IDA sub_4824C0 L41-88.  Los tres accesos estaban mal:
    //
    //   IDA                                        port anterior
    //   CharacterAttribute + 11                    ca + 0x00
    //   v6  = *(__int16 *)(CharacterMachine + 536) cm + 0x86*2 = +268
    //   v28 = *(__int16 *)(CharacterMachine + 604) cm + 0x97*2 = +302
    //
    // 268 y 302 son los INDICES de short (536/2 y 604/2) usados como offset de
    // BYTE: los dos accesos leian a la mitad de la direccion correcta, asi que
    // el tipo de arma salia basura, el scan no encontraba nada y la funcion
    // devolvia -1 siempre — por eso la municion no se auto-equipaba.
    //
    // Reparto de slots (confirmado por sub_4824C0 y por CreateArrow 0x474370):
    //   CharacterMachine + 536 (slot 0) -> BALLESTA (136-142, 144, 146)
    //   CharacterMachine + 604 (slot 1) -> ARCO     (128-134, 145)
    // y la municion va al slot que queda libre: Arrows (143) con arco, Bolts
    // (135) con ballesta.
```

### Línea 52 en `Item_FindElfWeaponInventorySlot` — antes de `int* piRow = &DAT_07ea9504;`

```cpp
    // Scan equipment table from DAT_07ea9504 downward (stride 0x11 dwords = 0x44 bytes per slot)
    // 8 rows x 8 columns, looking for first slot matching weaponGroup with durability > 0
    //
    // BUG-FIX 2026-05-03: el original usa `if ((int)piRow < 0x7ea9328) return -1;`
    // — una direccion absoluta del binario fuente, que en nuestro build no
    // significa nada.  Reemplazado por un contador explicito.
    // 2026-08-22: ese contador era de 7 y son 8 columnas
    // ((0x7EA9504 - 0x7EA9328) / 68 + 1 = 8).  Se nota tambien en `col`, que
    // arranca en 7 y tiene que llegar hasta 0.
```

## `src/Item/Item_Inventory.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// 2026-04-30: la versión 0.97 tenía estos handlers como stubs vacíos / no-op,
// por lo que el server enviaba el packet F3/10 (inventory-snapshot) y el cliente
// lo descartaba — ningún item aparecía en el grid.
//
```

### Línea 191 en `WriteEquipmentSlot` — antes de `BYTE* cm = (BYTE*)(uintptr_t)DAT_07cf1ffc;`

```cpp
    // BUG-FIX 2026-05-01: el código previo mezclaba DWORD indexing y byte
    // offsets. Resultado: Type se escribía bien (al inicio), pero Durability
    // (cm[off + 0xE] como DWORD index) caía en posición wrong dentro del
    // siguiente slot. RenderBrokenItem leía durability=0 → "Light Saber (0/69)"
    // siempre.
    //
```

### Línea 223 en `WriteEquipmentSlot` — antes de `*(int*)(slot + 4)    = (int)optByte;`

```cpp
    // 2026-05-08: BUG-FIX +N glow on EQUIPMENT slots.
    // Same fix as AddItemToGrid: store the RAW Option byte (level<<3 encoded)
    // so the render-side shift `(Level >> 3) & 0xF` extracts the correct +N.
    // Previously we stored decoded `level` (0-15) → render shifted again →
    // +9 → +1, no glow. Equipment uses the same render path (sub_4E38B0 /
    // Render_HotbarItems3D → RenderItem3D → RenderObjectScreen).
```

### Línea 231 en `WriteEquipmentSlot` — antes de `*(BYTE*)(slot + 27)  = byteHi;`

```cpp
    // FIX 2026-07-21 — Option1 llevaba `optByte` (= Attribute1, el byte de
    // nivel).  Los flags de EXCELLENT viven en Attribute2 (= byteHi = Item[3]):
    // ItemConvert (0x47B910) hace `iItemExcel = Attribute2 & 63`, y esa es la
    // MISMA mascara 0x3F que testean CalcMaxDurability (0x4C45C0) y
    // RenderItemInfo sobre ip->Option1.
    // Sintomas: los anillos/pendants excellent mostraban durabilidad maxima 15
    // puntos MENOS (no se aplicaba el bonus +15 de excellent) y les faltaba el
    // prefijo "Excelente" en el nombre.
    // Las opciones excellent SI se veian porque salen de Special[]/SpecialValue[],
    // que ItemConvert llena aparte — y a esa funcion los argumentos le llegaban
    // bien (ver la llamada ItemConvert(slot, optByte, byteHi) mas abajo).
```

### Línea 300 en `WriteEquipmentSlot` — antes de `slot->Durability = durability;`

```cpp
            // BUG-FIX 2026-05-01: Durability era el byteOpt (level/option byte)
            // → broken-item warning falso. Ahora usa el byte real del packet.
```

### Línea 315 en `WriteEquipmentSlot` — antes de `}`

```cpp
            // 2026-05-08: BUG-FIX item +N glow.
            // sub_4E38B0 pasa `*(int*)(slot+4)` (= slot->Level int) como param_6
            // a RenderItem3D → RenderObjectScreen, que extrae el level con
            // `(param_6 >> 3) & 0xF`. Si Level está pre-decoded (0-15), el
            // shift en RenderObjectScreen produce (9>>3)=1 → no glow.
            // ItemData_FillStats(level) acaba de setear Level = decoded —
            // override aquí con el byteOpt RAW para preservar el shift chain.
```

## `src/Item/Item_InventoryGrid.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Item_InventoryGrid.cpp
//
// Extracted from stubs_game.cpp.  This module owns the item-grid operations
// used by inventory, warehouse, trade, and chaos-mix panels.
//
// Every entry point retains its original IDA symbol/address in its leading
// comment.  No 5.2 logic was imported during this extraction.
```

### Línea 188 — antes de `extern "C" int __cdecl Item_CompareForTradeHistory(const BYTE* p, const BYTE* n);`

```cpp
// (El port anterior llamaba a CompareItems con tipo/nivel/durabilidad en
// vez de los dos registros, asi que la comparacion era basura.)
```

### Línea 285 en `CheckInventorySpace` — antes de `if (*(short*)cell == -1) {`

```cpp
                        // 2026-08-24 FIX (issue #15, "la jewel solo aplicaba en la 1er celda"):
                        // aca decia `|| *(int*)(cell + 0x38) <= 0`, o sea contaba la celda como
                        // VACIA cuando su Key era 0. Pero AddItemToGrid deja Key=0 en todas las
                        // celdas NO primarias de un item multi-celda (usa Key=1 solo para marcar
                        // la primaria), asi que de un item 2x2 tres de sus cuatro celdas se
                        // reportaban libres. IDA sub_4D5D70 L47 mira UNICAMENTE el Type:
                        //     if ( a3[34 * v13 + 34 * v14] == -1 )  ++v20;
                        // El campo Key solo gatea el RENDER (sub_4E38B0 L60), no la ocupacion.
```

### Línea 459 en `CalculateInventoryValue` — antes de `int itemVal = Item_CalculateValue((void*)pCell, 0);`

```cpp
                            // BUG-FIX 2026-04-26 (audit #3): ItemValue(item, sellMode=0).
                            // Antes el stub recibía (durability, 0, item, 0) → arg order roto.
```

## `src/Item/Item_InventoryRender.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Item_InventoryRender.cpp
//
// Extracted from stubs_game.cpp.  Owns the inventory/equipment render passes.
// Every entry point retains its original IDA symbol/address in its leading
// comment; this is a file reorganization only.
```

### Línea 37 — antes de `static ITEM* GetEquippedInventoryItem(int /*index*/)`

```cpp
// 2026-08-08: devolvía `((ITEM*)OffsetInventoryItems) + index` — la QUINTA copia
// del mismo error. `OffsetInventoryItems` es el pool del grid 8×8 y su índice de
// celda es `slot - 12` (AddItemToGrid:396): `[0..11]` son las CELDAS 0..11 del
// inventario visible, NO los wear slots. Los 12 slots de equipo viven sólo en
// `CharacterMachine + 536 + 68*slot`.
// Era el fallback de `GetPanelEquipmentSourceItem`, así que se disparaba justo
// al desequipar (slot de CM en -1) y dibujaba en la caja lo que el jugador
// tuviera en las primeras celdas del inventario — la poción en los pants, un
// casco en los anillos, etc.
```

### Línea 145 — antes de `void __stdcall RenderEquipmentBox(void) {`

```cpp
// RenderEquipmentBox @ 0x004E25A0 — port FIEL desde IDA (2026-05-02 v2).
//
// Renders los slot decoration backgrounds (placeholder icons como helmet
// outline, weapon outline, etc.) en la INVENTORY panel. Cada slot dibuja
// un bitmap a la posición correcta. Las posiciones y texturas vienen del
// IDA decompile de RenderEquipmentBox.
//
// Layout verificado contra IDA (mismas posiciones que RenderEquipment3D):
//   byte 1080 → (15, 46)   40×40   tex 275  (Pendant)
//   byte 1012 → (115, 46)  60×40   tex 274  (Wings)
//   byte 672  → (75, 46)   40×40   tex 263  (Helmet, skip if class==3)
//   byte 740  → (75, 89)   40×60   tex 264  (Armor)
//   byte 808  → (75, 152)  40×40   tex 265  (Pants/Boots — bottom-CENTER)
//   byte 536  → (15, 89)   40×60   tex 266  (WeaponL)
//   byte 604  → (134, 89)  40×60   tex 276  (WeaponR)
//   byte 876  → (15, 152)  40×40   tex 267  (Gloves — bottom-LEFT)
//   byte 944  → (134, 152) 40×40   tex 268  (Boots — bottom-RIGHT)
//   byte 1148 → (55, 89)   20×20   tex 269  (Ring1)
//   byte 1216 → (55, 152)  20×20   tex 270  (Ring2)
//   byte 1284 → (115, 152) 20×20   tex 270  (Necklace, same tex as Ring2)
//
// FIX 2026-05-02 v2: la versión vieja usaba `_DAT_00552c04/c10/...` globals
// que no estaban inicializados con los valores correctos → boxes se pintaban
// en posiciones equivocadas (cuadro gris al lado de armor que el user reportó).
// Ahora todas las posiciones son hardcoded literales matching IDA exactamente.
```

### Línea 191 en `RenderEquipmentBox` — antes de `if (!CharacterAttribute ||`

```cpp
    // 2026-08-08 FIX (el MG seguía mostrando la caja del casco): el gate leía
    // `*(short*)CharacterAttribute` — o sea un SHORT en el offset 0 — en vez del
    // byte de clase. Per IDA RenderEquipmentBox (0x4E25A0 L178) es
    //     if ( (*(BYTE *)(CharacterAttribute + 11) & 7) != 3 )
    // Mismo campo que usa RenderEquipment3D para saltear el ITEM del casco.
```

### Línea 202 en `RenderEquipmentBox` — antes de `SetEquipmentSlotPlaceholderColorForIndex(3);`

```cpp
    // 2026-08-08 FIX (el recuadro de la armadura salía 10 px más arriba y las
    // pants parecían caerse por debajo): acá se restaba `_DAT_00552488` (=10) a
    // la Y de la CAJA. Ese -10 es del ITEM, no del recuadro:
    //   RenderEquipmentBox  (0x4E25A0 L248-252): `v57 = v49 + 89.0;`
    //                                            RenderBitmap(264, v56, v57, 40, 60)
    //   RenderEquipment3D   (0x4E3100):          `syc = v45 + 89.0 - 10.0;`
    // O sea la caja va en +89 y el item se dibuja 10 px más arriba dentro de
    // ella (que es lo que hace RenderEquipmentPart3D, y eso queda igual).
```

### Línea 260 — antes de `void __stdcall RenderEquipment3D(void) {`

```cpp
// RenderEquipment3D @ 0x004E3100 — port FIEL desde IDA decompile (2026-05-01).
//
// Layout en CharacterMachine (byte offsets verificados contra IDA + capturas
// del cliente original 2026-05-01 v2 — labels corregidos):
//   536  WeaponL    (15, 89)  40×60
//   604  WeaponR    (134,89)  40×60
//   672  HELMET     (75, 46)  40×40   ← antes mal-labeled "Pendant"
//   740  Armor      (75, 79)  40×60   ← y = 89-10 = 79
//   808  PANTS      (75,152)  40×40   ← antes mal-labeled "Boots"
//   876  GLOVES     (15,152)  40×40   ← antes mal-labeled "Pants"
//   944  BOOTS      (134,152) 40×40   ← antes mal-labeled "Gloves"
//   1012 Wings      (115,46)  60×40
//   1080 PENDANT/Pet (15, 46) 40×40   ← antes mal-labeled "Helmet"
//   1148 Ring1      (55, 89)  20×20   via RenderEquipmentPart3D(9)
//   1216 Ring2      (55,152)  20×20   via RenderEquipmentPart3D(10)
//   1284 Necklace   (115,152) 20×20   via RenderEquipmentPart3D(11)
//
// Cada ITEM (68 bytes) en CM:
//   +0  Type (short),  +4 Level (DWORD),  +27 Option (byte),  +56 Durability (int)
//
// Nota: la versión previa tenía labels mezclados (decía "Armor 0xfd" pero leía
// byte 1012 = Wings real, etc) y screen positions desde DAT_ globals con valores
// distintos a los de IDA. Esto causaba que en la pantalla aparecieran items en
// posiciones equivocadas (helmet apilado con rings, pendant donde casco, etc).
```

### Línea 339 — antes de `// RenderItems3D @ 0x004E38B0 (~130 lines) — render 3D item models in inventory grid`

```cpp
//
// 2026-09-26: aca habia una copia bajo el nombre RenderItemsBoxes.  Las dos
// implementaciones son equivalentes; se deja una sola, con el nombre de IDA.
```

## `src/Item/Item_LegacyGridInsert.cpp`

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

### Línea 47 — antes de `extern "C" int __cdecl ConvertItemType(BYTE* Item);  // declared above`

```cpp
// InsertInventoryItem @ 0x004CC660 — InsertInventoryItem(ITEM* Inv, int Width,
//   int Height, int Index, BYTE* Item, bool First)  (1945 bytes, IDA).
//
// IDA decomp ships with a `// local variable allocation has failed` warning,
// so the visible body is mostly the anti-tamper hash-table tail (CharacterMachine
// ref-count + XOR encrypt around `First` branch). The actual grid-placement
// core was lost in the decompile — but cross-referencing the call sites
// (Net_Process.cpp cases 0x22 / 0x32 / 0x39 / F3-14) plus the parallel 0.52
// port `AddItemToGrid` in Item_Inventory.cpp gives us the real semantics:
//
//   - Convert item type (Item[0] + 2*(Item[3] & 0x80)). Skip if 0xFF (empty).
//   - Look up width/height from ItemAttribute. For 0.97 we accept the
//     simplification of 1×1 placement when ItemAttribute table is unavailable.
//   - Compute slot offset: (Index * sizeof(ITEM_RAW)) within the inventory
//     buffer. ITEM_RAW stride is 0x44 (= 68 bytes) per the binary format that
//     server emits in opcode 0x22 / 0x32 / 0xF3-14.
//   - Copy 12 bytes of raw item data into the slot (matches what
//     Net_Process.cpp:2807-2814 already does for case 0x22).
//   - If `First==false`, bump CharacterMachine hash-table ref count (anti-
//     tamper, no-op per project policy).
//
// 2026-05-08: ported as the canonical entry point. Net_Process inline copies
// keep working unchanged, but anything that needs the IDA name (and the
// signature taking grid-W/H plus First flag) can now use this.
```

### Línea 85 — antes de `static const int kEquipOffsets[12] = {`

```cpp
            // 2026-08-22 FIX (items que desaparecian al equipar/desequipar):
            // esto hacia `Inv + slotIndex * 0x44`, que es la CELDA `slotIndex`
            // del grid 8x8 — no el wear slot.  Al equipar los pants (slot 4) se
            // borraba la pocion de la celda 4, o sea la primera fila del
            // inventario; el server nunca se enteraba, de ahi que el item
            // "volviera" al reentrar (llega el F3/10) y que rechazara cualquier
            // drop en esa celda.
            //
            // Los 12 wear slots NO tienen espejo en `OffsetInventoryItems`:
            // viven en `CharacterMachine + 536 + 68*slot`.  Es la sexta copia de
            // este mismo error (ver CLAUDE.md, 2026-08-08 g-bis).
```

### Línea 172 — antes de `BYTE durability = Item[2];                    // raw durability byte`

```cpp
    // 2026-05-08: this was the bug that caused inventory items received via
    // packets 0x32/0x39/F3-14 to populate the data array but not appear.
```

## `src/Item/Item_LegacyHelpers.cpp`

### Línea 1 — antes de `// stubs_helpers.cpp`

```cpp
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

## `src/Item/Item_LegacyLinker.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 41 — antes de `extern "C" BYTE OffsetWarehouseItems[];`

```cpp
// IDA: CloseInventoryRelatedWindows (0x004CBA60)
// Cierra tienda / baul / chaos / trade / eventos y vacia sus pools.
//
// 2026-09-11: unica implementacion de 0x4CBA60.  Habia dos ports vivos y
// distintos: esta y `CloseInventoryRelatedWindows` (UI_LegacyGameHelpers.cpp, que ahora delega
// aca).  La lista de flags es la del disassembly (0x4CBB46..0x4CBD2F):
//   ShopOpened, byte_7EAA132, RepairEnable_0 (DWORD en 0x07EAA134),
//   WarehouseOpened, byte_559F5F, dword_7EAA14C, ChaosMixOpened, TradeOpened,
//   EventWindowOpened, g_bEventChipDialogEnable (0x07EAA128),
//   g_shEventChipCount (0x07EAA12C), g_bServerDivisionEnable/Accept.
// Esta version limpiaba antes DAT_07e11d14 como "RepairEnable" y dos alias del
// panel del Golden Archer (DAT_07e5ba80 / DAT_07e11e1c): ninguno de los tres
// tiene xrefs en IDA.  La anterior "desviacion" GoldenArcherOpenType = 0 era en
// realidad g_bEventChipDialogEnable, o sea parte del original.
//
// Pools (0x4CBD36..0x4CBD9C), Type = -1 y Key (+0x38) = 0:
//   120 registros de la tienda, 32 de la Chaos Machine, 32 de `Inventory` y
//   32 de OffsetTradeItems; y si byte_7EAA0E8 == 1, tambien los 32 del trade
//   del otro jugador (word_7E11F78 / dword_7E11FB0).
// Los DAT_ de esos bucles son alias de campo de los pools (globals.h).  El de
// 120 es el pool de 0x07EA5B30, que en el arbol es OffsetWarehouseItems; la
// tienda del port usa ademas su propio ShopItems (en IDA es el mismo pool),
// asi que se limpian los dos.
```

### Línea 245 en `ItemConvert` — antes de `if ((wType >= 387 && wType <= 390) || wType == 19 || wType == 146 || wType == 170) {`

```cpp
    // AUDITORIA 2026-07-20 — itemExcel ahora es FIEL a IDA ItemConvert (0x47B910).
    // El original hace exactamente esto y nada mas:
    //     iItemExcel = Attribute2 & 63;
    //     if (Type 387..390 || 19 || 146 || 170) iItemExcel = 0;
    // Aca habia DOS lineas de mas que no existen ni en IDA ni en el DLL de
    // inyeccion (verificado en Source/Client/Main/Item.cpp, que reemplaza
    // ItemConvert entero y tampoco las tiene):
    //
    //   1) `if (bExtOption) itemExcel = 1;`  ← la peor: forzaba el flag excellent,
    //      y de ahi `levelAddValue += 25`, inflando RequireStrength/Dexterity/
    //      Energy y los bonus excellent de damage/defense de CUALQUIER item con
    //      el ext-byte puesto.
    //   2) `if (Type 416..423 || >= 448) itemExcel = 0;`  ← ceroeaba de mas.
    //
    // `bExtOption` se conserva: NO alimenta la matematica de stats, pero si el
    // color del item (ip->Color / byColorState), que lo consumen
    // Render_PlayerEquipment y HUD_Pass4.  Ese bloque es otro injerto de origen
    // distinto y se audita aparte.
```

### Línea 512 — antes de `int __cdecl ItemValue(ITEM* ip, unsigned int goldType) {`

```cpp
// 2026-05-08: ItemValue delegates to Item_CalculateValue. Previously
// returned 0 unconditionally → all sell-price calculations in Item_Click
// Handler / RenderItemInfo / shop UI yielded zero gold. Delegate to the real
// impl in stubs_helpers.cpp.
```

## `src/Item/Item_Move.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 9 — antes de `void Net_SendSmallPacket(const BYTE* pkt, int totalLen);`

```cpp
// SendRequestEquipmentItem @ 0x0043C250 — Equipment move / item drag
//
// 2026-05-08 BUG-FIX MAYÚSCULO (round 2): la versión anterior mandaba C1
// plano. El servidor exige Encrypt=1 para opcode 0x24 (HackPacketCheck.txt
// línea 38: "36 * 1 0 0 0"), así que cualquier packet 0x24 que llegue como
// C1 es kickeado por `CHackPacketCheck::CheckPacketHack` → CloseClient →
// "Has sido desconectado del servidor".
//
// La solución correcta es usar `Net_SendSmallPacket` (Game_SceneUpdate.cpp:112),
// que es el mismo helper que login (F1/01) y combat usan. Hace:
//   1. Chain XOR con s_LoginKey (i=3..len)
//   2. Stomp pkt[1] = DAT_05826ceb++ (serial counter — server valida que sea
//      monotónico vía CSerialCheck::CheckSerial)
//   3. CSimpleModulus encrypt vía CSimpleModulus_Encode
//   4. C3 wrap: [C3][outerLen][encryptedBlob]
//   5. Send vía socket con WSAEWOULDBLOCK queue
//
// Layout plaintext esperado por Net_SendSmallPacket: [C1][size][head][payload].
// Para 0x24 PMSG_ITEM_MOVE_RECV (ItemManager.h:39):
//   struct {
//     PBMSG_HEAD header;        // C1 : len=11 : 0x24
//     BYTE SourceFlag;          // 0=inventory, 1=trade, 2=warehouse, 3=chaos-box
//     BYTE SourceSlot;
//     BYTE ItemInfo[4];         // [type, optByte, dur, typeHi|exc]
//     BYTE TargetFlag;
//     BYTE TargetSlot;
//   };
// Total: 3 + 1 + 1 + 4 + 1 + 1 = 11 bytes.
//
// `pItem` (= pPickedItem = DAT_07e91350) es un buffer ITEM 0x44 bytes; los
// primeros 4 son el wire format del server (per Recv_Inventory):
//   pItem[0] = type byte 0
//   pItem[1] = optByte (level<<3 | skill | luck | options)
//   pItem[2] = durability
//   pItem[3] = type hi nibble | excellent
//
// `iSrcType` mapeo: 0=inventory, 1=trade, 2=warehouse, 3=equipment-direct-equip.
// `iDstIndex` codifica destino: para inventario es slot index puro (0..63);
// los callers en stubs_game.cpp:1296/1320 ya pasan el slot encoded.
// C++-linkage forward decl matching Net.h:89.
```

## `src/Item/Item_SpecialOption.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 8 — antes de `extern "C" void __cdecl PlusSpecial(unsigned short *Value, int Special, DWORD Item);`

```cpp
// Item_GetDefenseWithSpecial @ 0x0047CFB0 (~17 lines) — get item special option value
// If item type==-1: return 0. Otherwise PlusSpecial(0x3f, item).
// IDA: sub_47CFB0 (0x0047CFB0)
// IDA sub_47CFB0: defensa del item (Value[9]) + PlusSpecial(63) (opcion de
// defensa adicional).  2026-09-12: faltaba el PlusSpecial, asi que el DefRate
// (Stats_CalcDefenseRate) salia sin ese bonus.  El primer parametro sigue sin usarse.
```

## `src/Item/Item_TypeLegacy.cpp`

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

## `src/Item/NPC_Data.cpp`

### Línea 5 — antes de `void __cdecl NPCName_LoadTextData(const char *path)`

```cpp
// IDA: FUN_0047D120
// Reads a text-format NPC name data file.
// Parser uses TextParser_GetToken (type 0=section/END, 1=record, 2=EOF).
// For each non-section record:
//   - First field (from initial TextParser_GetToken before the loop): NPC type_id (float→int)
//   - Two unused fields skipped (TextParser_GetToken × 2)
//   - Name string from TextParserTokenString copied into NPC name table
//
// NPC name table layout:
//   Base: &DAT_07cf2000  (actually byte array; EditMonsterNumber = current count)
//   Each entry: stride 0x36 (54 bytes)
//     [0x00] = type_id (char from first float read before loop)
//     [0x01..] = name string (memcpy from TextParserTokenString)
// Count (EditMonsterNumber) incremented after each entry write.
//
// Sentinel: when TextParser_GetToken returns 0, compare TextParserTokenString with DAT_00559088
//   (the "END" marker); if equal, break inner loop and process next section.
// PORT FIEL de OpenMonsterScript (0x47D120) — 2026-07-24.
// Antes escribia un type_id placeholder 0 y leia el nombre de TextParserTokenString
// (buffer equivocado) → tabla inutil.  Layout real por entrada (stride 0x36):
//   [0]     Type   = 1er token (columna 1 del archivo = entity type)
//   [1..32] Name   = 3er token (columna 3; el 2do se saltea)
// Formato NPCName.txt: "<Type> <idx> \"<Name>\"" por linea, hasta "end"/EOF.
// GetToken (TextParser_GetToken) saltea el header "//..." y las comillas.
```

## `src/Item/NPC_ModelLoad.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// NPC_ModelLoad.cpp
//
// Extracted from stubs_game.cpp.  Owns runtime NPC model/texture loading.
// The original IDA symbol/address remains in the function comment.
```

## `src/Item/Quest_UIState.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.
```

### Línea 11 — antes de `void __fastcall CSQuest_clearQuest(int param_1) {`

```cpp
// CSQuest::clearQuest @ 0x00401960 — cierra la ventana de quest.
// 2026-08-21: acá había un "CharSelect_SendClickPacket" que SÓLO mandaba el
// paquete.  Le faltaban las dos cosas que realmente cierran el panel, así que
// el botón X (y cualquier otro camino de cierre) no hacía nada: el flag
// +0x1C87F seguía en 1, GetScreenWidth seguía devolviendo 450 y el panel
// quedaba dibujado para siempre.  IDA:
//     *(_BYTE *)(This + 116863) = 0;
//     CloseInventoryRelatedWindows();
//     send([C1][03][31]);
// El 0x31 (49) va como C1 plano — HackPacketCheck le da Encrypt = 0.
// IDA: CSQuest::clearQuest (0x00401960)
```

## `src/Item/Skill_Data.cpp`

### Línea 114 en `Skill_LoadBMD` — antes de `int off = 0;`

```cpp
    // 2026-08-22: aca solo se llenaba SkillAttribute2 (= DAT_07cf1ff8, 0x7CF1FF8,
    // confirmado por xrefs de IDA).  SkillAttribute (0x07D29D20) quedaba en
    // ceros, y es la que leen TODOS los consumidores: RenderItemName case 795,
    // RenderItemInfo y GetSkillInformation.  Por eso el nombre de los orbes en
    // el suelo salia solo como "Jewel" — el sprintf es
    // `"%s %s"` con `&SkillAttribute[8*(5*Level+150)]` y GlobalText[102]
    // ("Jewel"), y el primer %s salia vacio.
```
