# Historial de comentarios: `src/Game/`

Comentarios de desarrollo movidos desde `src/Game/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Game/Game_CharSelectTick.cpp`

### Línea 65 en `Game_CharSelectTick` — antes de `CameraAngle[2] = -45.0f;  // yaw = -45° (iso rotation around Z)`

```cpp
        // BUG-FIX 2026-06-28: DAT_083a42c0 es DWORD& → `= -45.0f` convertía el
        // float a ENTERO -45 (0xFFFFFFD3) que leído como float es NaN. El yaw
        // NaN propagaba a AngleMatrix→VectorIRotate→CameraPosition (escena negra).
        // Escribir el float directamente (bit-pattern 0xC2340000), igual a IDA.
```

### Línea 141 en `Game_CharSelectTick` — antes de `if (g_bUseChatListBox == 1) {`

```cpp
        // ── Reposición per-frame del ChatListBox ────────────────────────────
        // FIX 2026-07-20: faltaba el `else` del `g_bUseChatListBox == 1`, y el caso
        // (-10, 81) estaba metido en la rama equivocada.
        //
        // Las 3 posiciones (verificadas en el binario en 0x5258D8: tres pares
        // de `push` que convergen en el mismo `call sub_40C690`) son:
        //     (186, 420)  sin paneles abiertos
        //     (  0, 420)  con inventario/character/trade/shop/etc. abiertos
        //     (-10,  81)  con el ChatListBox APAGADO → historial arriba-izquierda
        // La estructura es idéntica a CheckFunctionButtons (0x4C04A0), que sí
        // está bien portada en src/Input/Input.cpp:388-433; me guié por esa.
        //
        // Sin el `else`, al apagar el recuadro (botón 2 del popup o F4) el
        // widget se quedaba clavado en (186, 420) y los mensajes seguían
        // dibujándose ahí abajo en vez de volver arriba a la izquierda: las
        // filas se posicionan SIEMPRE en `this[11]+10, this[12]-13*n-16`
        // (IDA sub_40D610), así que mover el widget es lo único que las mueve.
```

### Línea 203 en `Game_CharSelectTick` — antes de `bool doLeaves;`

```cpp
    // Gate de MoveLeaves (hojas / lluvia / niebla). IDA 00524E30 L508-527:
    //
    //   if ( World ) {
    //       if ( World == 2 ) { if ( HeroTile == 3 || HeroTile >= 10 ) goto LABEL_108; }
    //       else if ( World != 3 && World != 7 && World != 9 && World != 10 ) goto LABEL_108;
    //   }
    //   else if ( HeroTile == 4 ) goto LABEL_108;
    //   MoveLeaves();
    //
    // O sea LABEL_108 es SALTEAR. El port tenia las condiciones de World 0 y
    // World 2 INVERTIDAS: corria solo en los casos en que IDA saltea, asi que
    // las hojas de Lorencia y Devias estaban al reves.
```

### Línea 227 en `Game_CharSelectTick` — antes de `{`

```cpp
    // 2026-05-03: per-entity animation tick RE-ENABLED. La concern de stack
    // corruption original venía de NULL-deref en hash table (HashTable_GetNode
    // returning NULL on key-mismatch). Con el sentinel hash setup ahora hay
    // un buffer válido siempre, y HashTable_GetIndex retorna -1 para que los
    // callers skip el deref.
    //
    // 2026-05-05: Wire MoveCharactersClient (per-frame entity tick que
    // llama MoveCharacterClient → MoveMonsterClient path-walker para cada entidad). Sin
    // esto los monsters/NPCs llegaban con packet 0x10 (target_grid set) pero
    // nunca se invocaba el path-walker, así quedaban quietos en su pos
    // inicial. El path-walker SÍ existe y funciona — solo faltaba wirear.
    // 2026-05-05: per-frame entity tick.
    //   MoveMonsterClient — path tick: pathfind (+0x306/7 target ≠ cached) y
    //                  advance waypoint cuando arrived. NO se llama para el
    //                  hero (Player_InputTick maneja su propio path/motion).
    //   MoveCharacterVisual — copia entity.action y world pos al model. SÍ para
    //                  todos los entities (incl hero). Sin esto el model
    //                  queda en posición inicial.
    //   CharacterAnimation — avanza entity[+0x108] (frame counter). Para todos.
```

### Línea 261 en `Game_CharSelectTick` — antes de `extern void __stdcall MoveBugs(void);`

```cpp
    // ── BUG-FIX 2026-07-16: MoveBugs FALTABA en char-select ─────────────────
    // IDA Game_CharSelectTick (00524E30 L531) llama MoveBugs(). Es el update que
    // hace fade-in del alpha de las entidades "bug" (Alpha() en MoveBugs L72) y
    // posiciona/anima las MONTURAS (Uniria bug=195 / Dinorant bug=267) siguiendo
    // al owner. Sin él, el alpha del mount queda en 0 → Calc_RenderObject lo
    // cullea (alpha < 0.01) → la montura nunca se dibuja. Verificado por diag:
    // el bug 267 existía en el pool pero Calc devolvía 0.
```

### Línea 271 en `Game_CharSelectTick` — antes de `DamageNumbers_Tick();`

```cpp
    // (2026-09-12: aca habia un `Character_UpdateAll()` = 0x479730, que es
    //  RenderSprites -- dibuja el pool de sprites y LES LIMPIA el flag.  En IDA
    //  solo lo llaman Game_RenderTick, Scene_Login y Scene_CharSelect; el tick
    //  del mundo (0x524E30) no.  Llamado aca dibujaba fuera del pase 3D y
    //  borraba los sprites antes del render real.  Game_RenderTick ya lo llama.)
```

### Línea 280 en `Game_CharSelectTick` — antes de `extern void __stdcall MoveParticles(void);`

```cpp
    // ── BUG-FIX 2026-07-15: MoveParticles (0x477090) FALTABA en char-select ──
    // IDA Game_CharSelectTick (00524E30 L539) llama MoveParticles() acá. Es el
    // update que decrementa el lifetime de las partículas y las despawnea. Sin
    // él, las partículas que spawnean las wings/armas/efectos de los personajes
    // se renderizaban cada frame (RenderParticles) pero NUNCA morían → se
    // acumulaban con blend aditivo → haces dorados saliendo de los bordes de la
    // pantalla, intensificándose progresivamente. El tick in-world
    // (Game_EnterWorldTick L310) sí lo llama; el de char-select no lo tenía.
```

## `src/Game/Game_EnterWorldTick.cpp`

### Línea 111 — antes de `static void Send_CharSelectPacket(void)`

```cpp
// Send the character-enter-world packet:
// [0xC1][len][0xF3] + username(10B, XOR) + padding(zeros, XOR) + slot_byte(XOR)
//
// BUG-FIX 2026-04-28: pkt[] era stack-buffer sin inicializar.  El loop XOR
// del slot byte hace `pkt[13] ^= key[13] ^ pkt[14]` y pkt[14] estaba en garbage
// stack-residual → cada call producía un cipher distinto del slot byte → el
// server descifraba un slot inválido y NO respondía con F3/03 (silent drop).
// Forzamos memset a 0 para que pkt[14] sea determinista (0) y el XOR final
// del slot byte sea estable.  Mismo fix aplicado al duplicado en case 0x19.
// 2026-05-05 BUG-FIX server kick post-F3/03 select-char:
// El encoding viejo usaba `pkt[i] ^= key[i] ^ pkt[i+1]` (NEXT byte) — pero
// el server (PacketManager.cpp::XorData) descifra con `pkt[n] ^= pkt[n-1]
// ^ key[n]` (PREVIOUS byte) iterando backward. Eso significa el encoding
// correcto del cliente es FORWARD usando `pkt[i-1]` (previous, que ya
// quedó encoded en el step anterior). Mismo patrón que la duplicate
// version en Game_CharSelectTick.cpp:108 y que Net_SendSmallPacket.
//
// Bug visible: server al recibir F3/03 select-char descifraba garbage,
// CGCharacterInfoRecv leía un nombre corrupto, GDCharacterInfoSend al
// DataServer fallaba (Char no existe), DataServer respondía DGCharacterInfoRecv
// con result=0 → CloseClient (DSProtocol.cpp:630). Eso es el FD_CLOSE.
//
// El flow alternativo es que el server SÍ procese el F3/03 con el nombre
// correcto (caso "mago" que se loguea OK) pero al final algún check
// secundario falla y kickea. La fórmula correcta abajo elimina la
// posibilidad de garbage en el descifrado.
```

### Línea 166 — antes de `static void Send_CharCreatePacket(void)`

```cpp
// Send the character-CREATE packet (2026-07-17):
//   [0xC1][15][0xF3][0x01][name(10)][Class] — server PMSG_CHARACTER_CREATE_RECV.
// Antes el OK de crear mandaba Send_CharSelectPacket (F3/03 = seleccionar char
// existente) → el server recibía un select de un char inexistente → FD_CLOSE.
// Class = page*16 (DB_CLASS_DW=0, DK=16, FE=32, MG=48) — la fórmula
// `page*0x10 + byte1` del IDA da ese valor.
```

### Línea 213 en `Game_EnterWorldTick` — antes de `DAT_083a4299 = 0;   // double-click flag`

```cpp
        // ── BUG-FIX 2026-04-25 ──────────────────────────────────────────────
        // Limpiar flags de click "stale" heredadas de la escena anterior.
        // En login/serverselect el usuario hace clicks rápidos para conectar;
        // si dos LBUTTONDOWNs caen <500ms aparte, Windows manda WM_LBUTTONDBLCLK
        // setteando DAT_083a4299=1.  Ese flag persiste hasta char-select y
        // cuando Mouse_Hover marca SelectedCharacter=0 (slot 0 hovered) la rama
        // DBLCLK del slot loop dispara → confirmed=true → state salta de 0x14
        // directo a 0x19 → entrada al mundo sin que el usuario haya clickeado
        // un slot.  Limpiamos también DAT_083a4124 por las dudas.
```

### Línea 306 en `Game_EnterWorldTick` — antes de `DAT_083a7ad0  = *(float*)&DAT_00561670;`

```cpp
        // Set camera to char-select world position (slot 5 of CameraWalk).
        // BUG FIX (igual que Game_SceneUpdate): DAT_00561664..0056167b son
        // aliases DWORD& sobre el storage float de CameraWalk_005615ec[30..35].
        // Si los asignamos a float& (DAT_083a7ad0/4/8, _DAT_083a4334) MSVC
        // hace int→float conversion. Forzamos lectura como float reinterpretando
        // el storage (mismo offset, distinto tipo).
```

### Línea 327 en `Game_EnterWorldTick` — antes de `{ extern void __stdcall MoveBugs(void); MoveBugs(); }`

```cpp
    // ── BUG-FIX 2026-07-16: MoveBugs FALTABA en el tick de char-select ──────────
    // Este ES el tick de char-select (Game_MainLoop dispatch state 4 → esta fn,
    // pese al nombre "EnterWorldTick"). IDA 0x521D80 L401 llama MoveBugs() acá,
    // primero. Hace el fade-in del alpha de las entidades "bug" (Alpha()) y
    // posiciona/anima las MONTURAS (Uniria bug=195 / Dinorant bug=267). Sin él,
    // el alpha del mount queda en 0 → Calc_RenderObject cullea → no se dibuja.
    // (La sesión previa lo puso en Game_CharSelectTick, que es el tick de state 5.)
```

### Línea 337 en `Game_EnterWorldTick` — antes de `Effect_TickAll();            // MoveEffects (0x0046B790)`

```cpp
    // ── BUG-FIX 2026-04-27: MoveParticles (MoveParticles) decrementa
    // lifetime de cada particle del pool DAT_07abf5f0. Sin esta llamada los
    // particles spawneados (lightning ELS=11, fire/smoke, etc.) se acumulan
    // forever → whiteout. Per IDA/5.2 RenderBlurs_RenderCharacterScene este
    // call se hace per-frame en MoveCharactersClient/MoveCharacterScene path.
    // MEJORA DEL DLL (no esta en IDA): el binario solo llama MoveParticles en
    // 0x005223EE; el DLL (Patchs.cpp MoveParticles_MoveCharacterScene) agrega
    // MoveEffects + MoveJoints para que el efecto de las alas se vea en
    // char-select.  Sin el tick de joints los de vida 0 no morian nunca aca.
```

### Línea 459 en `Game_EnterWorldTick` — antes de `// ── BUG-FIX 2026-04-27 ────────────────────────────────────────────`

```cpp
        // ── BUG-FIX 2026-07-17: las flechas de cambio de clase se MOVIERON al
        // bloque `else if (DAT_005616b0 != -1)` (create/name view). IDA
        // Game_EnterWorldTick: el input del create-panel (flechas de clase,
        // OK/Cancel, nombre) corre cuando `dword_5616B0 >= 0`, no en la lista.
        // Antes vivían acá (== -1) → tras clickear NEW CHARACTER (que setea
        // 5616B0>=0) este bloque se saltea → las flechas nunca corrían.
```

### Línea 466 en `Game_EnterWorldTick` — antes de `// Entity slot click detection (5 slots)`

```cpp
        // ── BUG-FIX 2026-04-27 ────────────────────────────────────────────
        // IDA original (00521D80) tiene el bloque "back/delete/Enter" dentro
        // de `if (DAT_005616b0 != -1)` (rama name-input), NO dentro de
        // `if (DAT_005616b0 == -1)` (rama lista). Estaba mal ubicado y eso
        // hacía que Enter en la lista (sin slot abierto para nombre) gatillara
        // el flujo de creación nuevo personaje incluso con slots llenos.
        // El bloque ahora vive en el `else if (DAT_005616b0 != -1)` más abajo.
```

### Línea 509 en `Game_EnterWorldTick` — antes de `bool selectClicked = false;`

```cpp
            // ── BUG-FIX 2026-04-25 ───────────────────────────────────────
            // Port previo perdía AMBOS guards (`if (!v103)` y `if (!v40)`):
            // el bloque que setea state=0x19 corría cada frame, y con
            // SelectedHero=-1 leía garbage memory (selSlot*0x394+0x1c0 con
            // selSlot=-1 cae en -0x1d4 desde la base) → triggeraba world-entry
            // sin click del usuario → char-select desaparecía tras 1s.
```

### Línea 536 en `Game_EnterWorldTick` — antes de `if (mouseX >= 441 - (int)DAT_005616a8 &&`

```cpp
                // 2026-09-12: el port sumaba el desplazamiento del panel en vez de
                // restarlo y dejaba el rect en 6 px de ancho -> el OK no respondia
                // (el Enter si, porque va por el otro camino).
```

### Línea 570 en `Game_EnterWorldTick` — antes de `else if ((int)DAT_005616b0 != -1) {`

```cpp
    // ── NAME-INPUT VIEW (slot reservado para nuevo char): b0 != -1 ────────────
    // IDA: `if (-1 < DAT_005616b0)`. Aquí Enter (DAT_055ca038) y los botones
    // back (mouseX 0xea..0x133) / OK (mouseX 0x14f..0x196) operan sobre el
    // diálogo de nombre del nuevo personaje. Antes de este fix vivían dentro
    // de `if (DAT_005616b0 == -1)` y disparaban con Enter en la pantalla de
    // lista, generando el bug "Enter activa New Character con slots llenos".
```

### Línea 661 en `Game_EnterWorldTick`

```cpp
// BUG-FIX 2026-07-17: F3/01 crear (no F3/03 select)
```

### Línea 753 en `Game_EnterWorldTick` — antes de `{`

```cpp
                // ── WORLD ENTRY: copy char data → transition to Loading ──────
                // (BUG-FIX 2026-04-25: en el port original este bloque estaba
                //  FUERA del switch → se ejecutaba cada frame → state 4→3→5
                //  cíclicamente, disparando Game_CharSelectTick que envía un
                //  F3 char-name packet sin MuEmu wrap → server FD_CLOSE.
                //  IDA original 0x00521D80:1150 pone esto DENTRO de case 25
                //  después del Pkt_Send y check de validCode.)
```

### Línea 817 en `Game_EnterWorldTick` — antes de `}`

```cpp
  // 2026-07-16 DIAG: bracket la sección UI de char-select
    // (NB: the world-entry transition block — SceneFlag=3, FUN_005102c0,
    //  FUN_00404c60(5) — used to live here, OUTSIDE the switch.  Eso era un
    //  port-bug: en el binario original ese código está DENTRO de case 25
    //  (= nuestro case 0x19), tras el Pkt_Send y el check de validCode.
    //  Al estar afuera, se ejecutaba cada frame → state 4→3→5 cíclico →
    //  Game_CharSelectTick disparaba unencrypted F3 → server FD_CLOSE.)
```

## `src/Game/Game_MainLoop.cpp`

### Línea 50 — antes de `#define ML_HEAP_CHECK 0`

```cpp
// 2026-08-17 — FRENO DE RENDIMIENTO, no es parte del port.
// ChkHeapPublic() llama _CrtCheckMemory(), que recorre TODO el heap de debug
// validando los guard bytes de cada bloque asignado. Con los modelos, texturas
// y el terreno cargados son decenas/centenas de miles de bloques, y abajo se
// invocaba 16 VECES POR FRAME → el chequeo solo puede costar más que el frame
// entero. El original no tiene nada equivalente: es instrumentación nuestra
// para cazar corrupción de heap (ver ChkHeapPublic en WinMain.cpp).
// Queda detrás de un switch, apagado por defecto. Poner en 1 para reactivarlo
// cuando haya que volver a rastrear corrupción de heap.
```

### Línea 77 en `Game_MainLoop` — antes de `{`

```cpp
    // 2026-05-05 diag: log entry rate-limited (per state)
```

### Línea 192 en `Game_MainLoop` — antes de `#define SCREENSHOT_DIR_DEVIATION 1`

```cpp
        // IDA 0x525D40 L308:
        //   sprintf(GrabFileName, "Screen(%02d_%02d-%02d_%02d)-%04d.jpg",
        //           st.wMonth, st.wDay, st.wHour, st.wMinute, GrabScreen);
        //
        // El port tenia "Screen %02d %02d %02d %02d - %04d" con st.wYear. Dos
        // bugs: (a) sin la extension .jpg, y el archivo lo escribe WriteJpeg
        // (WriteJpeg, calidad 100), asi que quedaba un JPEG sin extension
        // que el explorador no reconocia; (b) con el ANO en vez de GrabScreen
        // el nombre solo cambiaba por minuto, asi que dos capturas en el mismo
        // minuto se pisaban. GrabScreen (DAT_083a42f0) lo incrementa
        // SaveScreen modulo 10000.
        //
        // DESVIACION DELIBERADA (pedido del usuario, 2026-09-20): el binario
        // guarda en la RAIZ del cliente -- GrabFileName no lleva ruta.  Para
        // no ensuciarla, las capturas van a "Screenshots/".  La carpeta se
        // crea una sola vez por sesion y, si no se puede crear, se cae a la
        // raiz, que es el comportamiento original.
        //
        // Se usa barra normal a proposito: fopen la acepta en Windows y es lo
        // que ya usa el resto del archivo (ver Monster_SaveSetBase mas abajo).
        //
        // SCREENSHOT_DIR_DEVIATION en 0 devuelve el comportamiento de IDA.
```

### Línea 280 en `Game_MainLoop` — antes de `GL_EndOpenGL();   // EndOpengl → pop MODELVIEW + PROJECTION (balancea el BeginOpengl)`

```cpp
    // IDA Game_MainLoop (0x525D40): BeginOpengl(0,0,640,480); glClear; EndOpengl();
    // BUG-FIX 2026-06-28: antes era GL_PopMatrixAll() (hack que popea 8×2=16 matrices)
    // sobre un stack con sólo 2 pushes → GL_STACK_UNDERFLOW (0x504) cada frame.
    // El balance correcto es EndOpengl (pop MODELVIEW + PROJECTION), igual a IDA.
```

### Línea 360 en `Game_MainLoop` — antes de `#if 0`

```cpp
    // ── CONNECTION CHECK ──────────────────────────────────────────────────────
    // BUG-FIX 2026-04-28: este check leía *(int*)(SocketClient + 8) y comparaba
    // con -1. En el original SocketClient era un Object* con un Type field en
    // +8; en nuestro port SocketClient es un buffer estático sin esa estructura
    // → el read devolvía garbage que a veces == -1 → disparaba 0x71 ConnLost
    // 100ms después de JoinMapServer, anulando todos los menús.
    // Real disconnect detection: WSA FD_CLOSE event en WinMain; ese path setea
    // DAT_055ca018 que ya bloquea Game_MainLoop al inicio.
```

### Línea 381 en `Game_MainLoop` — antes de `#if 1`

```cpp
    // ── LIVECLIENT KEEPALIVE (opcode 0x0E) ────────────────────────────────────
    // BUG-FIX 2026-04-28: server MuEmu (Protocol.cpp:68 CGLiveClientRecv)
    // espera C1/0E cada ~1-3 seg después de OBJECT_LOGGED. Sin este packet,
    // el server timeout-ea y manda F1/02 sub=0 (Exit). Era la causa del
    // "se cierra sin cartel" después de entrar al mundo.
    //
    // Packet: [C1] [0x0B] [0x0E] [TickCount:DWORD] [PhysiSpeed:WORD] [MagicSpeed:WORD]
    // Total = 11 bytes.
    // 2026-04-29: keepalive 0x0E re-habilitado. Con el send() hook en MuEmu.h/cpp
    // toda send call ahora se auto-encripta si el primer byte es C1/C2/C3/C4 plain.
    // 2026-05-04: gate cambiado de `state==5` a `state>=4 || sub==7`. El
    // server cerraba el socket DURANTE la carga de mapa (state=5 sub=7 →
    // sub=0 transición), 5+ s de silencio del cliente. Ahora keepalive
    // empieza desde char-select (state=4) y sigue mientras carga el mapa.
```

## `src/Game/Game_SceneUpdate.cpp`

### Línea 95 en `LoginScene_ApplySafeObjectAnim` — antes de `}`

```cpp
        // BUG-FIX 2026-07-13: NO spawnear efectos aquí. Este call site duplicaba
        // el spawn de Entity_SpawnEffects: los barcos/objetos ya lo reciben desde
        // el pass de render (Terrain_Render.cpp:149, tras Entity_PrepareRender que computa
        // los bones world-space frescos). Aquí, Calc_RenderObject NO refresca bien el
        // bone scratch → los 2 flares del barco salían con bones stale (mismo valor
        // para los 3 barcos) → aparecían flotando en el centro/al lado. El original
        // llama Entity_SpawnEffects UNA vez por entidad, desde el render. Removido.
```

### Línea 370 en `Game_SceneUpdate` — antes de `if (g_HasConnectServer) {`

```cpp
        // ── Flujo ConnectServer (2026-07-15) ──────────────────────────────────
        // Si server.cfg trae 2 líneas (línea 1 = ConnectServer), conectamos YA
        // al ConnectServer para recibir la lista real + el load. A diferencia
        // del GameServer, el ConnectServer NO responde JoinServer al conectar:
        // espera nuestro request C1 04 F4 02 (enviado en FD_CONNECT) y responde
        // con F4/04 (nombres) + F4/02 (load). Al elegir server mandamos F4/03 y
        // el redirect nos lleva al GameServer. Si NO hay línea 2, queda el flujo
        // directo clásico (conectar al elegir server).
```

### Línea 399 en `Game_SceneUpdate` — antes de `{`

```cpp
        // Spawn background world objects.
        // ── BUG-FIX MASIVO (2026-04-20) ───────────────────────────────────────
        // Valores canónicos de Ghidra @ 0x0051F900 (MoveLogInScene) líneas
        // 100-260. El port previo tenía inventados los cálculos de pos y
        // había perdido el reset de `rot` antes del mu banner (0xA2) y del
        // sky4 (0xA3), haciendo que esos 2 últimos rendereasen con
        // rot_z=180 heredado → banner volteado, sky4 volteado.
        // También el port metía Z=180 a los 3 ships cuando original es Z=0,
        // y posición del primer barco estaba como (-79,158,102) cuando la
        // real es (-700,700,0).
```

### Línea 410 en `Game_SceneUpdate` — antes de `float pos[3], rot[3], scale = 1.0f;`

```cpp
            // ── VALORES CANÓNICOS (2026-04-21) ─────────────────────────────
            // Restaurados desde Ghidra/IDA @ 0x0051F900 líneas 150-235.
            // CreateObject aplica scale override vía
            // byte_4FFAA4[type-60] cuando SceneFlag==2||4:
            //   type 60  (ship)    → scale 0.8
            //   type 160 (sky)     → scale 0.0438
            //   type 161 (wave)    → scale 0.8
            //   type 162 (banner)  → scale 0.6
            //   type 163 (sun)     → scale 3.0
            // No hacer hacks de scale ni de rot aquí — si algo no renderiza,
            // el bug está upstream en el pipeline (no en los valores).
```

### Línea 477 en `Game_SceneUpdate` — antes de `*(float*)(base + 0x734) = 0.6f;`

```cpp
            // Entity 2 (elf) — BUG-FIX: Y era -802, el canónico es -770.
            // Ghidra @ 0x0051F900 línea 239: CharactersClient[2].Position[1] = -770.0
```

### Línea 549 en `Game_SceneUpdate` — antes de `_DAT_083a42d4 = 0; _DAT_083a42d8 = 0; _DAT_083a42dc = 0;`

```cpp
        // Camera initial position from server-slot table
        // ── BUG FIX: DAT_005615ec/f0/f4/f8/fc/600 son aliases DWORD& sobre el
        // storage float CameraWalk_005615ec[]. Si los leemos como int y los
        // asignamos a un float& (DAT_083a7ad0/4/8 y _DAT_083a4334), MSVC hace
        // conversión int→float que destruye el bit pattern (200.0f leído como
        // 0x43480000 → asignado como float 1128792064.0f). El log CAM mostró
        // CurrentCameraAngle=(3.27e9, 0, 3.24e9) y CurrentCameraPosition[2]=1.13e9.
        // Forzamos lectura via cast a float* para reinterpretar correctamente.
```

### Línea 579 en `Game_SceneUpdate` — antes de `MoveParticles();`

```cpp
    // IDA Game_SceneUpdate (0x51F900) llama MoveParticles() cada frame. Nuestro
    // Particle_Update() es en realidad Trail_RenderAll (0x46C3E0, mal nombrado) y
    // NO decrementa el lifetime de las partículas. MoveParticles (0x477090)
    // sí las tickea/expira. Faltaba acá → las partículas del hada (Particle_Spawn
    // 1175 + sparkle 1150) se acumulaban forever additive → whiteout dorado en el
    // server-select. Mismo fix que Game_EnterWorldTick.
```

### Línea 657 en `Game_SceneUpdate` — antes de `UIChatLogWindow_AddText((const char*)&DAT_083a7c74, GlobalText[470], 1);`

```cpp
                    // IDA 0x0051F900 L445-446: these are GlobalText[470]/[471],
                    // not independent empty buffers.  Same fix as L612-613.
```

### Línea 845 en `Game_SceneUpdate` — antes de `int totalLen = pos;`

```cpp
                // ── LoginKey chain XOR ──────────────────────────────────────
                // ANTES: aplicado inline aquí (solo F1/01).
                // AHORA: aplicado universalmente en Net_SendSmallPacket /
                // Net_SendLargePacket — para que TODOS los paquetes salientes
                // (F1/05 HWID incluido) lleven el chain, igual que el companion
                // (Mu-linux-97K Source/Client/Main/Protocol.cpp:983 ExtractPacket).
                // Sin chain en F1/05 el server veía subh=0x7D en vez de 0x05 y
                // dejaba HardwareID="" → F1/01 devolvía code=05 (HWID empty).
```

### Línea 863 en `Game_SceneUpdate` — antes de `{`

```cpp
                // 2026-07-25 (#1): log de cada intento de login para diagnosticar
                // el "primer enter = dato mal, segundo enter entra".  Correlacionar
                // con "F1/01 LOGIN-RESULT code=..." en Net_Process: si el intento N
                // manda len iguales pero el server responde fail en el 1ro y OK en
                // el 2do, el problema es el primer paquete (serial/encriptación) o
                // un estado stale de conexión, no las credenciales.  NO logueamos
                // la password, solo longitudes + serial actual.
```

### Línea 990 en `Game_SceneUpdate` — antes de `BYTE pkt[4];`

```cpp
                    // Send 0xC1/0xF3/0x00 char-list request (4 bytes).
                    //
                    // BUG-FIX (2026-04-25 v2): el companion SÍ aplica chain XOR
                    // a paquetes C1, no solo a C3.  CPacketManager::ExtractPacket
                    // (Source/Client/Main/PacketManager.cpp:438) llama XorData
                    // con start=end+1=3 (C1 header de 2 bytes), end=size, sobre
                    // el buffer completo ANTES de evaluar si se hace C3 wrap.
                    // Los C1 conservan el header pero el chain XOR ya fue
                    // aplicado.  Solo los C3/C4 entran al branch de re-encrypt.
                    //
                    // Antes: removí el chain XOR pensando que C1 path no lo
                    // usaba → server recibía `C1 04 F3 00` plain, hacía reverse
                    // XOR, veía subop corrupto → descarta silencioso (13s gap
                    // en log sin FD_READ).
                    //
                    // Bug original era simplemente OOB en pkt[4]: el formula
                    // correcta es `pkt[i] ^= pkt[i-1] ^ key[i]` — para
                    // totalLen=4 una sola iteración, `pkt[3] ^= pkt[2] ^ key[3]`.
                    // No hay acceso fuera del buffer.
```

## `src/Game/PathFinder.cpp`

### Línea 741 — antes de `extern "C" int __fastcall PathPQueue_GetCount(void *_this, void * /*edx*/)`

```cpp
// CONSTRUCCION E INICIALIZACION DEL CONTEXTO PATH  (2026-08-17)
//
// Hasta ahora el contexto (DAT_05826df4) se reservaba en WinMain con
// `malloc(0x420)` + memset, y por eso el vtable de la cola de prioridad en
// +0x414 quedaba NULL: PATH_FindPath (PATH::FindPath) crashea al llamarlo, y de
// ahi venia el `pfReady = false` forzado en stubs_externs.cpp, que obliga a usar
// el A* sustituto. Estas dos funciones portan lo que faltaba.
```

### Línea 813

```cpp
// ZzzAI::InitPath (0x0043F2D0) NO va aca: ya estaba portada, y correctamente,
// en stubs_externs.cpp (PathFinder_ResetContext). La llama OpenFont (World_Init) desde
// Scene_Intro, igual que en el binario. Definirla de nuevo aca daba LNK2005.
```

## `src/Game/Player_InputTick.cpp`

### Línea 24 — antes de `#define g_MouseOnWindow DAT_07d78094`

```cpp
// 2026-04-30: Bottom-bar HUD button hit-test.
// Los rectángulos salen de HUD_Pass5.cpp:264-296 (las mismas coordenadas que ya
// se usan para los tooltips de hover y el resaltado de "panel abierto"). Con un
// LButton release (no drag), toggles the corresponding panel flag.
//
// Gated by:
//   - sólo en estado in-game (SceneFlag == 5)
//   - released-click signal (DAT_083a413c) so holding doesn't repeat
//   - limpiar DAT_083a413c después de consumirlo, para que otra parte de la UI no lo maneje dos veces
//
// En el binario 0.97k original esto estaba inline dentro del ruido anti-tamper de
// sub_004B82xx; acá lo reimplementamos limpio porque el camino click→toggle
// es lo que realmente hace funcionar los botones del HUD.
// 2026-05-04: MouseOnWindow (per IDA Player_InputTick:416,566 + sub_402F40:9):
// flag que se setea en cada frame si el mouse está sobre algún panel de UI abierto. Se usa para gatear
// el GroundClick / walker de movimiento, así clickear adentro de un panel no hace
// que el jugador camine hacia esa posición de pantalla.
// 2026-09-09: `MouseOnWindow` es UN SOLO global del binario, 0x07D78094 (=
// DAT_07d78094).  Estaba partido en dos: el port fiel de `sub_4E6550`
// (CheckInventory) escribia DAT_07d78094 con los seis rects de panel
// -- inventario, tienda, baul, ChaosMix, trade y ventana de evento -- y el gate
// de `Attack` (IDA L1330) leia este `g_MouseOnWindow`, una reimplementacion
// propia que NO cubre ninguno de esos seis.  Ahora es un alias del global real.
```

### Línea 60 — antes de `extern "C" int g_ChatLB_MouseOnWindow;`

```cpp
// 2026-07-20: el ChatListBox publica su propio hit-test acá (definido en
// src/UI/ChatListBox.cpp).  Su tick (slot 5 → slot 7) corre ANTES que esta
// función dentro del mismo frame (Game_CharSelectTick líneas 217 y 298), así
// que el latch está fresco.  Sin esto, clickear dentro del recuadro del chat
// mandaba a caminar al personaje: el slot 7 escribía un `MouseOnWindow` local
// de ChatListBox.cpp que no leía nadie.
```

### Línea 112 en `MouseOnWindow_Update` — antes de `btnX = (int)((float)g_GuildCreatorScratchX + 100.0f);`

```cpp
        // 2026-08-26: mismo fix que el render — IDA `sub_4E4760` L446 pone el
        // segundo boton en [origin+100, origin+170), no en +20+100.
```

### Línea 136 en `MouseOnWindow_Update` — antes de `static char s_lastMenuOpen = 0;`

```cpp
    // 2026-05-05: Skill bar expanded list (cells at y=370..411 cuando
    // DAT_07db870c=1, el user expandió el menú con click en el icono central).
    // Sin este check, click en una skill cell del menú expandido cae en zona
    // libre del world → Player_InputTick lo procesa como move click → hero
    // camina al lugar del cell además de cambiar la skill.
    //
    // 2026-05-05 (followup): Chat_InputTick (corre ANTES) resetea DAT_07db870c
    // a 0 cuando user clickea una cell. Entonces cuando llegamos acá, el flag
    // ya cambió. Usamos un latch del frame anterior para que el click "tail"
    // siga viendo el menú como abierto.
```

### Línea 177 — antes de `enum HudPanelTail { TAIL_GUILD, TAIL_PARTY, TAIL_CHARACTER, TAIL_INVENTORY_CLOSE };`

```cpp
// entonces el panel que acababa de abrir).  2026-09-18: reemplaza a
// HUD_CloseNpcWindowsIfAny / HUD_CloseInventoryFamilyFromUI, que mandaban 0x31
// para todo e impedian abrir el panel.
```

### Línea 222 — antes de `extern "C" void HUD_BottomBarButtons_HitTest(void);`

```cpp
// Botones de la barra inferior.  En el binario esto vive dentro de
// `Chat_InputTick` (0x4B14F0); aca corre desde Player_InputTick, que se ejecuta
// antes de la logica de click al mundo.
//
// 2026-09-04 -- reescrito contra IDA.  La version anterior era una
// reimplementacion ("clean reimplementation ... covers the same observable
// behavior") con tres desviaciones:
//   * el boton de inventario gateaba con `HUD_IsInventoryFamilyActive()`, que
//     incluye CharacterOpened -> con el panel de personaje abierto, clickear
//     inventario lo CERRABA en vez de abrirlo.  En el original los dos paneles
//     conviven (por eso GetScreenWidth devuelve 260 justo para esa combinacion).
//   * usaba `DAT_083a413c` (latch de click soltado) con deteccion de flanco
//     propia; IDA usa `MouseLButtonPush` (DAT_083a4124) y lo CONSUME poniendolo
//     en 0, que es lo que evita el auto-repeat.
//   * le faltaban los sonidos 25/28 al cerrar, el `PartyOpened = 0` del boton de
//     guild, el `GuildOpened = 0; PartyOpened = 0` al abrir inventario, y el
//     gate de entrada que apaga toda la fila mientras hay un modal abierto.
//
// Rects (IDA L1130, L1455, L1775, L2078):
//   guild      (582..634, 459..477)
//   party      (348..372, 452..476)
//   personaje  (379..403, 452..476)
//   inventario (410..434, 452..476)
```

### Línea 331 — antes de `extern char g_PadBeforeChatMode[64];`

```cpp
// 2026-04-30: hotkeys de UI por frame para el HUD in-game (C/V/I).
// En el binario 0.97k original, los botones de la barra inferior (y estos
// atajos de teclado) invierten los flags de un byte DAT_07eaa11x que gatean cada
// bloque de render del HUD. La rutina dedicada que hacía esto estaba enterrada en
// 0x004B8xxx anti-tamper hash-table noise; this clean reimplementation
// cubre el mismo comportamiento observable para el usuario.
//
// Se suprime mientras haya algún modo de entrada de texto activo (chat, IME, texto de login)
// para que tipear letras en el chat no invierta paneles sin querer.
// 2026-05-04: defensive guard — GuildInputEnable/d71 (ChatMode/IME) get corrupted
// a 0xFF (-1 con signo) por ALGÚN código, poco después de abrir el inventario. El
// writer is hard to find via grep (no literal -1 store).  As a defense, clamp
// cualquier valor que no sea {0,1} a 0 al inicio de cada llamada a Player_InputTick Y logueamos
// las primeras veces que vemos la corrupción, para poder encontrar la fuente.
```

### Línea 360 en `ClampChatModeIME` — antes de `{`

```cpp
    // 2026-07-27: detector de la transición 0→1 de ChatMode. El clamp de arriba
    // sólo atrapa valores >1, pero un `1` espurio es un valor VÁLIDO ("chat on")
    // → abre la caja de chat/whisper con un carácter suelto (el bug del "whisper
    // con la letra l" que aparece cada tanto, típicamente tras abrir tiendas).
    // Logueamos el tag del punto del frame donde se encendió para ubicar al
    // escritor real.
```

### Línea 436 en `HUD_HotkeyTick` — antes de `if (DAT_07eaa11b ||                      // TradeOpened`

```cpp
    // IDA Chat_InputTick L3976-3985: ANTES de mirar cualquier hotkey de panel,
    // el original corta la funcion entera con esta lista:
    //
    //   TradeOpened || GuildCreatorOpened || GuildInputEnable
    //   || ErrorMessage == 126 || ErrorMessage == 152
    //   || g_bEventChipDialogEnable
    //   || *(BYTE*)(g_csQuest + 116863) == 1
    //   || g_bServerDivisionEnable
    //
    // 2026-08-21 se habia portado SOLO el termino de la ventana de quest
    // ("sin el gate se podia abrir el inventario encima del panel de quest --
    // los dos se dibujan en x=450").  El mismo razonamiento vale para el resto
    // de la lista, que es la que ya usa el handler de la barra inferior.
    //
    // 2026-09-20: faltaba g_bEventChipDialogEnable (el Golden Archer), y eso
    // causaba dos sintomas.  Con la ventana abierta, la V dibujaba el
    // inventario ENCIMA del panel (los dos van a x=450); y al volver a
    // apretarla, HUD_PanelTail97k -> CloseInventoryRelatedWindows (0x4CBA60
    // L154) limpia g_bEventChipDialogEnable sin avisarle al server.  El server
    // se queda con Interface.use != 0 y rechaza /move con el mensaje 65,
    // "You cannot move right now" (Move.cpp L181-185) -- y no se recupera,
    // porque el 0x31 que manda SendMove al caminar esta gateado por ese mismo
    // flag que la V ya puso en cero.
    //
    // Con el gate puesto, la ventana solo se cierra por su X o caminando, que
    // son los dos caminos que si mandan el 0x31 (igual que el DLL, que para
    // eso hookea SendMove en 0x00492AD2).
```

### Línea 537 — antes de `// Helper: manda el buffer por el socket, con fallback a la cola de WSAEWOULDBLOCK`

```cpp
// IDA: Player_InputTick — Player_InputTick (0x004acef0, 1688 lines)
//
// Procesador de input del jugador por frame. Se llama desde el camino de render del HUD/UI en cada frame.
// Responsibilities:
//   1. Cooldown gate (DAT_07e11d1c must be <= 0x1e)
//   2. Second-password auto-fill from hover entity name
//   3. Camera update + facing angle packet [0xC1][0x18][0x66]
//   4. Sub-tick via CheckGate
//   5. Movement debounce (DAT_07e11d28 >= DAT_00559bec and !DAT_07e11dc0)
//   6. Animation state exit: swimming, normal walk, cancel
//   7. Click sobre mob/jugador (SelectedCharacter): pathfind + Combat_SendMovePathPacket
//   8. Click sobre NPC (SelectedNpc): pathfind alternativo
//   9. Click sobre objeto especial (SelectedOperate): pathfind + lookup de entity_type
//  10. Click en suelo (ray cast RenderTerrain + RenderTerrainTile): chequeo de terreno + pathfind
//  11. Actualización de estado de entidad: Combat_DispatchHeroSkillAttack
//  12. Atributo de terreno bajo el cursor → DAT_07e118e8
//
// Los bloques de ofuscación por HashTable repartidos por la función (~70 % del código) se omiten
// per project policy (see CLAUDE.md §Anti-tamper).
//
// Key globals:
//   DAT_07abf5d8          — local player entity ptr
//   DAT_07abf5d0          — entity array base (stride 0x394)
//   DAT_07cf1ffc          — g_CharData (XOR-encoded char-select data block)
//   DAT_05826e08          — frame counter (float-compatible tick)
//   _DAT_00552890         — speed scale (dt multiplier)
//   _DAT_00552b6c         — max facing dt threshold
//   _DAT_00552928         — min entity speed for facing packet
//   DAT_00559bec          — movement debounce threshold
//   DAT_07e11d28          — movement debounce counter
//   DAT_07e11db8          — contador de pasos (tiene que ser > 0x27 para el facing)
//   SelectedCharacter          — hover mob/player entity index
//   SelectedNpc          — hover NPC entity index
//   SelectedOperate          — hover special-object index
//   SelectedItem          — hover ground item index
//   DAT_083a2378          — special-object entity pointer table (stride 3*int)
//
// Packet formats (all XOR-encoded before send):
//   Facing:       [0xC1][0x07][0x0F] + encoded direction byte
//   Walk/swim:    [0xC1][0x11] + grid_x,grid_y
```

### Línea 578 — antes de `static void SendPacket(const char *buf, unsigned int len)`

```cpp
// Helper: manda el buffer por el socket, con fallback a la cola de WSAEWOULDBLOCK
// BUG-FIX 2026-04-29: server log mostró `[SocketManager] Protocol header
// error (Header: 41)` — 0x41 = lo que el server-side decrypt produce cuando
// recibe nuestros bytes plain como si fueran cipher. Causa: este helper
// NUNCA llamaba MuEmu::EncryptSend. Server con ENCRYPT_STATE=1 decripta todo
// el stream → packets que mandamos plain salen como garbage → kick.
// El primer F3/03 (CharSelect) funcionaba porque va por OTRO path con
// EncryptSend (Game_CharSelectTick). Movimiento y swim packets desde aquí
// rompían la sesión.
```

### Línea 628 en `Player_ProcessInput` — antes de `HUD_HotkeyTick();`

```cpp
    // 2026-04-30: el procesamiento de hotkeys de UI va PRIMERO, para que los toggles funcionen incluso
    // cuando los gates de abajo saldrían temprano (p.ej. durante un cooldown).
```

### Línea 631 en `Player_ProcessInput` — antes de `MouseOnWindow_Update();`

```cpp
    // 2026-05-04: poblar el flag MouseOnWindow (per IDA) ANTES de la lógica de
    // GroundClick, así clickear adentro de un panel no hace caminar al jugador.
```

### Línea 634 en `Player_ProcessInput` — antes de `// ── Guard: entity visibility / renderable flag ─────────────────────────────`

```cpp
    // 2026-05-20: in-game unificamos el latch viejo de hover con la captura de UI
    // capture result. Inventory / character / chat render paths still poke
    // DAT_07d78094 directo y lo puede dejar pegado, lo que bloquea el movimiento
    // even when the mouse is no longer over a panel. For world input, only
    // sólo debería importar la captura de UI del frame actual.
    // 2026-09-09: aca habia `DAT_07d78094 = (g_MouseOnWindow != 0) ? 1 : 0;`,
    // que in-game PISABA el flag que ya habia puesto CheckInventory por los
    // paneles.  Sintoma: con la Chaos Machine abierta, mover items disparaba el
    // camino de ataque -> sin mana -> busca pocion -> cartel GlobalText[474]
    // ("Los items no pueden ser utilizados mientras usas el baul o durante
    // trade").  Ahora los dos nombres son la misma memoria y no hay que copiar.
    // 2026-09-04: los botones de la barra inferior se atienden desde
    // `Chat_InputTick` (0x4B14F0), que es donde los tiene el binario.
```

### Línea 680 en `Player_ProcessInput` — antes de `int histSlot = (int)DAT_00559cc4;`

```cpp
        // 2026-08-21: el port indexaba con SelectedCharacter (SelectedCharacter, que
        // llega hasta 399) sobre una tabla de 5 entradas → escribía 0x40 bytes
        // hasta ~100 KB fuera del global cada vez que se hacía click derecho
        // sobre un jugador con el chat abierto.
```

### Línea 701 en `Player_ProcessInput` — antes de `bool bHeadTrackActive = false;`

```cpp
    // La cadena ya estaba entera salvo el PRODUCTOR:
    //   · `MoveCharacterVisual` (0x4520C0) interpola cada frame
    //       HeadAngle[j] = TurnAngle2(HeadAngle[j], HeadTargetAngle[j],
    //                                 FarAngle(HeadAngle[j], HeadTargetAngle[j]) * 0.2)
    //   · `BMD_Animation` (0x440060) rota el hueso de la cabeza con
    //     HeadAngle[0]/[1] (grados → radianes, * 0.017453294).
    // Nadie escribía HeadTargetAngle del héroe, así que quedaba en 0 y el pj
    // miraba siempre al frente de su cuerpo.
    //
    // El port anterior había degradado justo las dos líneas del cálculo:
    // `GetScreenWidth()` — que es **GetScreenWidth**, no "frame time" — con el
    // resultado descartado, y `CreateAngle(0,0,0,0)` (CreateAngle) con ceros.
```

### Línea 756 en `Player_ProcessInput` — antes de `{`

```cpp
    // ── WALKER (corre cada tick, INDEPENDIENTE de gates) ─────────────────────
    // BUG-FIX 2026-05-01: el walker estaba adentro del gate `bec <= d28`,
    // pero `bec` se setea a `wpCount*3+4` cada vez que Combat_SendMovePathPacket envía un
    // packet de movimiento. Para wpCount=5 → bec=19 ticks (760 ms). Eso
    // throttleaba el walker a 1.3 calls/sec — el hero "se movía por zonas".
    //
    // 2026-05-05: además debe ir ARRIBA del gate `DAT_07d78094` (que se setea
    // cuando user hover sobre skill bar). Sin esto, hover sobre skill detenía
    // el walker mid-path → hero parado en el lugar pero anim de walk seguía
    // corriendo. El walker debe ejecutarse SIEMPRE; solo el envío de packets
    // y el procesamiento de NEW clicks debe gated.
```

### Línea 772 en `Player_ProcessInput` — antes de `bool isIdle = (ent[748] == 0);`

```cpp
            // BUG-FIX 2026-05-03: el chequeo isIdle DEBE ir ANTES de SetPlayerWalk.
            // Si está idle (sin path activo), NO queremos que SetPlayerWalk setee
            // walk action (action 0x0d) cada frame. Antes el orden era:
            //   SetPlayerWalk (set walk) → check isIdle → si idle: set 1 (idle)
            // → action cambia walk↔idle cada frame → frame counter reset cada
            // tick → render frozen en frame 0.
            // IDA gatea todo este walker con Hero+748. Los contadores de waypoint
            // son internos a MovePath y no hay que usarlos para enganchar la posición
            // de mundo mientras el runner de camino está inactivo.
```

### Línea 790 en `Player_ProcessInput` — antes de `*(unsigned char*)(ent + 0x354) = 0;   // cur_wp`

```cpp
                    // BUG-FIX 2026-05-03: al llegar al destino, resetear
                    // wp_count + cur_wp para que isIdle (línea 319) sea true
                    // en el frame siguiente. Sin esto, isIdle queda en false
                    // (wp_count != 0), el walker sigue corriendo cada frame
                    // ejecutando SetPlayerWalk (sets walk action) → SetPlayerStop
                    // (sets idle action) → frame counter reset cada tick →
                    // player FROZEN en pose de walk frame 0.
```

### Línea 800 en `Player_ProcessInput`

```cpp
// 2026-05-05: move_pending,
```

### Línea 804 en `Player_ProcessInput` — antes de `*(unsigned char*)(ent + 748) = 0;`

```cpp
                    // IDA Player_InputTick L399: `*(_BYTE *)(v0 + 748) = 0;`
                    // Es la UNICA escritura a +748 que tiene esa funcion en el
                    // binario y faltaba.  Sin ella el walker nunca se apaga: el
                    // flag queda en 1 para siempre y el camino se regenera sin
                    // pasar por idle (medido: 22 fines de camino contra 2
                    // arranques).
```

### Línea 812 en `Player_ProcessInput` — antes de `DAT_07e11dbc = (int)*(float*)(ent + 36);`

```cpp
                    // IDA L401: `dword_7E11DBC = (__int64)*(float *)(v0 + 36);`
                    // — es el FACING del héroe, no un timestamp. El port tenía
                    // `DAT_05826e08` (WorldTime), que dejaba basura en el campo
                    // que después lee la rotación por octante.
```

### Línea 859 en `Player_ProcessInput` — antes de `}`

```cpp
                            // TODO(paquete): el binario avisa acá al server con
                            // PMSG_ACTION_RECV (`C1:18`, Protocol.h:55 —
                            // {BYTE dir; BYTE action; BYTE index[2]}) llevando el
                            // octante como `dir`. El layout exacto del buffer en
                            // el decompile está entrelazado con el ruido de
                            // hash-table (la key de 32 bytes v233..v256), así que
                            // queda sin enviar hasta confirmarlo por disassembly:
                            // sólo afecta a que OTROS jugadores vean la rotación,
                            // y un paquete mal formado desconecta (ver la entrada
                            // "Desconexiones: serial de packets" de CLAUDE.md).
```

### Línea 932 en `Player_ProcessInput` — antes de `if (DAT_07e11d30 != 0 || g_MouseOnWindow != 0)`

```cpp
    // ── Salida temprana si el movimiento está bloqueado o la UI activa (post-walker) ──
    // 2026-05-05: el gate se movió a DESPUÉS del walker, así el walker siempre avanza
    // incluso con DAT_07d78094 seteado (mouse sobre la barra de skills). Sin esto,
    // hover over skill icon froze hero mid-walk with looping anim.
```

### Línea 939 en `Player_ProcessInput` — antes de `bool walkerIdle = (((unsigned char*)DAT_07abf5d8)[0x356] == 0);`

```cpp
    // ── Movement debounce gate (controla envío de packets/clicks, NO walker) ─
    // 2026-05-03: relax el gate cuando el walker está idle (wp_count == 0).
    // Antes el gate era estrictamente time-based (~1.2 sec entre clicks).
    // Si user clickea rápidamente, los clicks se descartaban silenciosamente.
    // Ahora: si idle, aceptar clicks de inmediato; si moviendo, mantener el
    // gate original para no spamear el server con paths intermedios.
```

### Línea 946 en `Player_ProcessInput` — antes de `DAT_07e11dc0 = 0;`

```cpp
    // [FIX #2 2026-06-30] DAT_07e11dc0 ("movement lock flag B") — per IDA solo lo
    // escriben Attack (0x49CC50) y Chat_InputTick (0x4B6630). Attack es stub vacío
    // en nuestro build y el port de Chat_InputTick omitió ese write, así que NADA
    // lo setea legítimamente → su valor fiel es 0. El runtime mostró -44 (corrupción
    // de un buffer adyacente que toggle con el inventario: cerrado=-44 bloqueaba el
    // gate de debounce). Forzamos 0 acá hasta portar Attack (que reimplementaría el
    // lock real) y/o encontrar el corruptor. Sin esto, el héroe no caminaba con el
    // inventario cerrado en Devias.
```

### Línea 957 en `Player_ProcessInput` — antes de `static bool s_clickCycleConsumed = false;`

```cpp
        // BUG-FIX 2026-04-30 (v2): un click = un GroundClick.
        //
        // Race condition entre WM_LBUTTONUP y este tick (PIT corre a 25 Hz / 40 ms).
        // Tres casos a manejar:
        //   1. Held click (DOWN > 40 ms): primer tick ve 4124=1 → procesar.
        //      Ticks siguientes mientras held: ya no debería dispararse otra vez.
        //   2. Tap rápido (DOWN+UP < 40 ms): nunca vemos 4124=1, solo 413c=1.
        //   3. Tras un Held: el UP setea 413c=1 — lo cual VOLVERÍA a disparar
        //      en el tick siguiente si solo miramos los flags directos.
        //
        // Solución: edge-guard que cubre un ciclo entero (DOWN→UP→idle).
        // El ciclo se abre con cualquier flag activo y se cierra cuando todo
        // queda idle (sin click held, sin latch, sin pending).
```

### Línea 971 en `Player_ProcessInput` — antes de `static bool s_clickStartedOnWindow = false;`

```cpp
        // 2026-05-05: trackea si el evento de click ABAJO pasó sobre una ventana.
        // Si sí, todo el ciclo del click (ARRIBA/soltar) también tiene que tratarse
        // como "click de panel" — aunque el usuario haya movido el mouse al mundo antes de soltar.
        // Sin esto, un click en un ícono de skill seguido de una deriva del mouse al
        // mundo antes de soltar disparaba un movimiento por el flanco de subida.
```

### Línea 978 en `Player_ProcessInput` — antes de `{`

```cpp
        // 2026-05-07 BUG-FIX: detectar entry a in-world (SceneFlag == 5)
        // y CONSUMIR los click flags stale del CharSelect click "Enter".
        // Sin esto, el latch DAT_083a413c=1 del click final en CharSelect
        // queda set al primer frame in-world → bClickEdge fires sin click
        // real → mob attack handler dispara → hero ataca al primer mob
        // visible al spawn. User reportó "aparece el char atacando cuando
        // entro al mundo sin haber clickeado nada" 2026-05-07.
        //
        // 2026-05-07 (followup): además wipear el entity pool DAT_07abf5d0
        // (excepto hero slot) porque CharSelect dejaba los slots de los chars
        // disponibles activos (slot[0]=1) con sus nombres en +0x1C1. Cuando
        // entrábamos al mundo, hover detect en FUN_004afdc0 leía esos slots
        // como entidades válidas y Target_Render mostraba sus nombres como
        // si fueran NPCs/players del mundo. User reportó "leo los nombres de
        // los personajes del select character" 2026-05-07.
```

### Línea 1010 en `Player_ProcessInput` — antes de `if (DAT_07abf5d8) {`

```cpp
                    // Clear hero action queue (+0x2ED) so secondary tick
                    // doesn't fire Action() with garbage. NO tocar +0x2EC —
                    // ése es un state flag (is_moving / in_action) que server
                    // maneja, no un "alive" flag (per IDA ReceiveAction:20
                    // setea a 0 durante acciones de entidades vivas).
                    //
                    // 2026-05-07: también resetear anim_state a idle (1) y
                    // path state. El hero entity hereda anim_state stale del
                    // CharSelect (donde se anima el preview en walk/idle) y
                    // sin reset queda walking-in-place al spawn del mundo.
```

### Línea 1031 en `Player_ProcessInput` — antes de `}`

```cpp
                    // 2026-05-07: NO wipear el entity pool aquí — Player_InputTick
                    // corre DESPUÉS que F3/03 JoinMapServer ya pobló el pool con
                    // viewport spawns (0x12/0x13). Wipearlo aquí borraba los mobs
                    // recién spawneados → user veía mundo vacío con hero walking
                    // in place. El wipe se hace en Net_Process F3/03 handler
                    // ANTES del OpenWorld load, donde es safe.
```

### Línea 1042 en `Player_ProcessInput` — antes de `bool bMousePush    = (DAT_083a4124 == 1);   // DOWN pulse this tick`

```cpp
        // 2026-05-07 (FINAL): semántica per IDA Player_InputTick:
        //   DAT_083a4124 = MouseLButtonPush (DOWN pulse, ONE-SHOT)
        //                  Lo setea WndProc en WM_LBUTTONDOWN si DAT_083a42c4==0.
        //                  IDA consume: `if (Push) { Push=0; v32=1; }`.
        //   DAT_083a42c4 = MouseLButton (estado de mantenido: se setea al bajar, se limpia al soltar)
        //   DAT_083a413c = MouseLButtonPop (set on UP no-drag)
        //
        // PUSH es one-shot — set en DOWN, consumed por nosotros aquí, no se
        // dispara again hasta el next DOWN.
        // HELD es continuous — refleja el real-time mouse button state.
        // POP es UP-edge — set en UP no-drag, consumed por dialog buttons etc.
        //
        // BUG previo: nuestro IsClickPushed() retornaba DAT_083a4124==1 que
        // permanecía true durante todo el hold. NO consumíamos en
        // InputTick → cada frame veía push=true → cualquier reset de
        // s_prevAnyClick disparaba edge espurio en hover.
```

### Línea 1075 en `Player_ProcessInput` — antes de `static int s_lastHoverMob = -1;`

```cpp
        // 2026-05-06: detectar cambio de hover target durante click held.
        // Si user click on mob A → mob A muere → user mueve mouse a mob B
        // sin liberar click, debería ser un new intent (new attack on B).
        // El cycle-consumed bloquea, así que reset cycle si target cambia.
        //
        // 2026-05-06 (followup): GATED POR bClickHeld. Sin esto, un
        // bClickLatched colgado (DAT_083a413c=1) más cualquier cambio de
        // hover (= mouse pasando sobre mob) reseteaba el cycle → bHoverActive
        // se volvía true sin click real → attack disparaba al pasar el mouse
        // sobre un mob. User reportó: "si le paso el mouse por encima ataca,
        // no si le hago click" 2026-05-06.
```

### Línea 1094 en `Player_ProcessInput` — antes de `static bool s_prevClickHeld = false;`

```cpp
        // 2026-05-06: detectar DOWN edge del click. Cada nuevo click DEBE
        // disparar nuevo cycle (incluso si user click rapidamente sobre el
        // mismo mob varias veces). Antes user tenía que mover mouse para que
        // funcionara cada attack — muy molesto.
```

### Línea 1105 en `Player_ProcessInput` — antes de `bool bClickEdge = bMousePush;`

```cpp
        // 2026-05-07 FINAL (matching IDA semantics):
        //   bClickEdge = bMousePush — el push pulse YA es one-shot per IDA.
        //   Después del click handler, lo CONSUMIMOS (DAT_083a4124 = 0).
        //   Próximos frames: push=0 hasta el next DOWN. NO se dispara en
        //   hover, NO se dispara espurio.
        //
        // Push semantics:
        //   - DOWN: WndProc sets DAT_083a4124=1, DAT_083a42c4=1.
        //   - InputTick this frame: bMousePush=true → process → consume (=0).
        //   - Held: 4124=0 (consumed), 42c4=1 (still held).
        //   - UP: WndProc clears 4124 (already 0), 42c4=0, sets 413c=1.
        //   - Next InputTick: bMousePush=false, all clean.
        //
        // El bClickLatched (POP/UP-edge) sigue funcionando como respaldo para
        // dialogs/menus que necesiten detectar UP. NO se usa aquí para
        // attack-arm (eso causaba el hover-attack bug).
```

### Línea 1122 en `Player_ProcessInput` — antes de `if (bMousePush && !g_MouseOnWindow) {`

```cpp
        // 2026-05-08 BUG-FIX MAYÚSCULO: solo consumir el push pulse si el
        // click NO está sobre una UI window. Si el cursor está sobre el
        // inventario / character panel / shop / etc., el click handler de
        // ESA window (FUN_004d23b0 invocado más tarde en el render pipeline
        // desde RenderInventoryWindow / RenderShopInterface / etc.) necesita
        // ver `DAT_083a4124 == 1` para detectar y procesar el click. Si lo
        // consumimos acá, el handler del UI ve 0 y el pickup/use jamás
        // dispara — síntoma observado: hovers funcionan (la rama hover de
        // FUN_004d23b0 no usa el flag), pero ningún click consigue mover ni
        // consumir items. `g_MouseOnWindow` lo setea HUD_HitTest_AllWindows
        // (líneas 65-90) basado en las flags de panel abierto + bounding box.
```

### Línea 1143 en `Player_ProcessInput` — antes de `{`

```cpp
        // 2026-05-07: durante los primeros 10 frames in-world, force bClickEdge=false.
        // Cubre el caso de WndProc dejando DAT_083a4124=1 colgado durante la
        // transición CharSelect → World, o cualquier edge espurio causado por
        // race condition en la inicialización del input system.
```

### Línea 1156 en `Player_ProcessInput` — antes de `bool bHoverActive = false;`

```cpp
        // 2026-05-07 SAFETY (mejorada): clear ent[0x2ed] (action queue) cuando:
        //   - NO hubo click edge este frame
        //   - El user NO está sosteniendo el botón izquierdo (bClickHeld=false)
        //   - Walker está idle (ent[0x356]==0) — sin path activo
        //
        // Match IDA Player_InputTick:599 que requires `m_bAutoAttack && Attacking==1
        // && SelectedCharacter!=-1` AND v32 (current click) para continuar combat.
        // Sin alguna de esas, bail (= no attack/action).
        //
```

### Línea 1166 en `Player_ProcessInput` — antes de `if (bClickHeld || bClickLatched) {`

```cpp
        // 2026-09-29: sacado el `&& !s_clickCycleConsumed`.  IDA
        // Player_InputTick L590-598 no tiene ningun concepto de "ciclo
        // consumido":
        //     v32 = 0;
        //     if ( MouseLButtonPush ) { MouseLButtonPush = 0; v32 = 1; }
        //     if ( MouseLButton )     { v32 = 1; }      // MANTENIDO
        // o sea con el boton apretado el click se procesa en CADA apertura
        // del gate, y el throttle es el propio MouseUpdateTimeMax -- que ya
        // gatea todo este bloque.  El guard era redundante con el gate y
        // ademas rompia su auto-regulado: como `MouseUpdateTime = 0` vive en
        // el camino de procesar el click, al bloquearse el contador no se
        // reseteaba nunca.
        //
        // Medido con sonda, caminando con el boton mantenido: al agotarse el
        // camino MouseUpdateTime valia 27/33/43 contra un MouseUpdateTimeMax
        // de 19/22/25.  O sea el gate se abria a tiempo (3*wp+4 < 4*wp) pero
        // el click no se procesaba, el camino no se encadenaba y se llegaba a
        // SetPlayerStop -- un frame con la pose de parado cada ~1.08 s.
```

### Línea 1187 en `Player_ProcessInput` — antes de `if (!g_MouseOnWindow) {`

```cpp
            // 2026-05-04: NO consumir DAT_083a4124 cuando el mouse está sobre
            // un panel — el render-phase de RenderCharacterInfoWindow / etc.
            // necesita ese flag para detectar clicks en sus botones (X close,
            // [+] stat add, etc.).  Si lo consumimos acá, los handlers de
            // panel ven `pressed=false` y nunca disparan.  Solo consumir
            // cuando el click ES para el ground (mouse fuera de panels).
```

### Línea 1198 en `Player_ProcessInput` — antes de `if (g_MouseOnWindow || s_clickStartedOnWindow) {`

```cpp
        // 2026-05-05: Si el mouse está sobre un panel (HUD bottom, skill
        // expanded list, panels right-side), forzar bHoverActive=false para
        // que los alt-targets (hover target, NPC click, tertiary) NO procesen
        // el click como movement. Antes user click en skill icon → bHoverActive
        // permanecía true → disparaba pathfind a stale hover target → hero
        // caminaba al lugar de un NPC/monster cercano.
        //
        // 2026-05-05 (followup): TAMBIÉN consume DAT_083a42c4 y reset
        // SelectedCharacter/4c/48 (hover targets) cuando hay click sobre window.
        // Sin esto, un hover target stale (NPC bajo el cursor del frame
        // anterior) hacía que líneas 757/801/831 dispararan pathfind aunque
        // bHoverActive=false, aunque DAT_083a42c4 conserve el latch del click.
```

### Línea 1220 en `Player_ProcessInput` — antes de `if (!bClickHeld && !bClickLatched) {`

```cpp
        // 2026-05-05: si el user NO está clickeando activamente (no held, no
        // latched), forzar DAT_083a42c4=0 también. Sin esto, un click anterior
        // que no se consumió bien puede dejar este flag activo después de
        // soltar el botón.
```

### Línea 1272 en `Player_ProcessInput` — antes de `if (act < 0x4C || act > 0x50)`

```cpp
                // DESVIACION DELIBERADA: la excepcion de IDA es 0x4E..0x50
                // (78..80 = stop/walk/run_TwoHandTwo).  La extendemos a
                // 0x4C (76) para cubrir tambien 76 y 77, Pegasus_fly y
                // Pegasus_fly_weapon -- el Dinorant volando en Tarkan e
                // Icarus.
                //
                // El rango bloqueante 0x22..0x5B empieza en attack_fist (34),
                // o sea esta pensado para no aceptar clicks durante un ataque.
                // Las acciones de caminar quedan debajo (las de montura son 32
                // y 33), y las tres de TwoHandTwo caen dentro solo por quedar
                // numeradas despues de los ataques -- de ahi que las exceptuen
                // a mano.  Con 76/77 se olvidaron, y son del mismo tipo:
                // animaciones de desplazamiento, no de ataque.
                //
                // Medido con sonda volando en Icarus: con la accion 77 el
                // click queda bloqueado (hover=16 pero reset=1), no se pide el
                // camino siguiente, el actual se agota y SetPlayerStop pasa un
                // frame por la pose de parado.  Ese frame es el salto que se
                // ve, y el ciclo se repite cada ~1.08 s.  Es el mismo artefacto
                // que Webzen describe en 5.2 ("애니메이션 튀는거", la animacion
                // salta) y que alla resolvieron dejando de usar 76/77.
                // Extender la excepcion es mas acotado: conserva la eleccion de
                // accion de SetPlayerWalk y solo destraba el input.
```

### Línea 1326 en `Player_ProcessInput` — antes de `if ((char)canAct != '\0' || bHoverActive) {`

```cpp
            // BUG-FIX 2026-04-28: CheckAttack retorna 0 cuando
            // no hay entidad bajo el mouse (SelectedCharacter == -1). El gate
            // original solo dejaba pasar entity-hover-clicks → ground-click
            // (clic en el suelo sin hover de entidad) NUNCA disparaba el
            // pathfind → hero no se movía nunca.
            // El IDA original probablemente separaba ground-click fuera de
            // este gate; aquí relajamos: si bHoverActive (click real), pasar
            // aunque canAct=0. Los handlers internos siguen gateados por
            // SelectedCharacter/4c/48/54 != -1, así que no disparan spurio.
```

### Línea 1364 en `Player_ProcessInput` — antes de `const bool bAutoAttackGoOn = m_bAutoAttack != 0          // m_bAutoAttack`

```cpp
                // ── Hover entity attack (SelectedCharacter valid) ─────────────────
                // 2026-05-06 (final): GATEAR POR bClickEdge (rising edge del
                // mouse press, capturado tanto desde bClickHeld como del
                // latch DAT_083a413c). Match IDA Player_InputTick que usa
                // `MouseLButton` raw — el ataque se arma SOLO en el frame
                // exacto donde el botón pasa de released → pressed.
                //
                // Bug original: bHoverActive era TRUE mientras bClickLatched
                // estuviera set (entre frames antes de consumirse), aunque
                // el user NO estuviera apretando el mouse. Cualquier cambio
                // de hover target durante esa ventana → attack disparaba al
                // pasar el mouse sobre un mob. User reportó: "si le paso el
                // mouse por encima ataca, no si le hago click".
                //
                // Con bClickEdge: dispara una sola vez por click. Sin posibili-
                // dad de spuriarse por latches stale, hover changes, etc.
                //
                // Filtro adicional: mobs MUERTOS (entity[+0x34e]==1) no son
                // targeteables.
                // 2026-09-16: IDA 0x004ACEF0 L590-603 no exige el flanco:
                //   v32 = MouseLButtonPush || MouseLButton;
                //   if ((!m_bAutoAttack || World == 6 || Attacking != 1 ||
                //        SelectedCharacter == -1) && !v32) goto LABEL_390;
                // O sea con el boton MANTENIDO se sigue atacando (el gate de
                // animacion de mas arriba marca el ritmo), y con m_bAutoAttack
                // el ataque continua al soltar mientras el objetivo siga
                // vivo (sub_4B0310 lo mantiene fijo).  Antes solo pegaba en el
                // frame del click.
```

### Línea 1407 en `Player_ProcessInput` — antes de `if (hoverEnt[0x2FD] != 0) {`

```cpp
                    // 2026-05-07: dead check usa SOLO 0x2FD per IDA ReceiveDie:18.
                    // El check viejo `0x2EC == 0` era WRONG: 0x2EC es un "state"
                    // flag que el server setea a 0 también en ReceiveAction y
                    // otros casos NO-muerte (per IDA ReceiveAction:20). Usarlo
                    // como "dead" filter rechazaba mobs vivos → click handler no
                    // armaba attack. User reportó que el ataque "a veces" no
                    // disparaba (cuando el mob estaba en mid-action).
                    // 2026-08-10: sacado el `|| hoverEnt[0x34e] != 0`. +0x34E es
                    // SafeZone, no dead: incluirlo volvía NO-targeteable a todo
                    // NPC parado en zona segura (o sea todos los del pueblo).
```

### Línea 1433 en `Player_ProcessInput` — antes de `TargetX = (DWORD)dstX;`

```cpp
                    // 2026-05-07 BUG-FIX: dst grid coords son del MOB target,
                    // NO `DAT_05826e08` (eso es g_AnimTick, tick counter).
                    // El bug viejo asignaba el tick counter como grid coord,
                    // entonces pathfind iba a un tile aleatorio basado en
                    // frame number → user reportó que click far mob no movía
                    // al hero pero hacía attack animation in place.
```

### Línea 1542 en `Player_ProcessInput` — antes de `const bool bClickNow = bClickEdge || DAT_083a42c4 != 0;   // MouseLButton`

```cpp
        // ── Alt-target: NPC/item (SelectedNpc != -1) ────────────────────────
        // 2026-09-04 (b): este bloque estaba DENTRO del guard
        //     if (SelectedOperate == -1 || (montado && !SafeZone)) { ... }
        // que envuelve las ramas de NPC / item / click al suelo.  O sea cuando SI
        // habia objeto seleccionado, el guard saltaba todo el bloque -- incluido el
        // propio manejo del operate.  Medido en debug.log: en el frame del click
        // `MOVEHOVER ... c54=23` no lo seguia ni `PIT GroundClick!` ni la sonda.
        // En IDA los dos son `if` HERMANOS y el de SelectedOperate va primero:
        //     if ( SelectedOperate != -1 ) { ... }
        //     if ( SelectedNpc != -1 )     { ... }
        // Objeto interactuable bajo el cursor (sillas, bancos, barandas,
        // orbes de Noria).  IDA 0x4ACEF0 L1133-1195.
        //
        // 2026-09-04 FIX: este bloque estaba como `else if (bClickEdge)`
        // del `if (!shiftHeld)` de abajo, o sea SOLO corria con Shift
        // apretado.  En IDA la cadena es secuencial y SelectedOperate se
        // chequea ANTES del ramo de movimiento por terreno:
        //     if ( SelectedOperate != -1 ) { ... goto LABEL_340/LABEL_312; }
        //     ...
        //     if ( GetAsyncKeyState(16) >> 8 != 0x80 ) { RenderTerrain(1); ... }
        // y las dos salidas del bloque saltan al final del tick, o sea
        // tienen PRECEDENCIA sobre el click al suelo.
        // IDA L590-598: las ramas de mobiliario, NPC e item corren con
        //     v32 = MouseLButtonPush || MouseLButton
        // o sea con el boton MANTENIDO tambien, igual que el click al suelo.
        // El port les exigia el flanco (bClickEdge): si el flanco se consumia
        // en un tick bloqueado por la animacion o el debounce, el click caia
        // al suelo con el item bajo el cursor y el heroe caminaba en vez de
        // levantarlo.  2026-09-18.
```

### Línea 1586 en `Player_ProcessInput` — antes de `TargetX = (DWORD)(int)(*(float*)(tgtEntityPtr + 0x10) * 0.01f);`

```cpp
                // 2026-09-04 FIX: TargetX/TargetY salen de la POSICION
                // DEL OBJETO, no del tile bajo el cursor.  IDA L1138:
                //     TargetX = (__int64)(o->Position[0] * 0.01);
                //     TargetY = (__int64)(o->Position[1] * 0.01);
```

### Línea 1626 en `Player_ProcessInput` — antes de `if (DAT_07eaa118 == '\0' && DAT_07eaa119 == '\0') {`

```cpp
                // 2026-05-06: bClickEdge en vez de bHoverActive — mismo fix
                // que el attack handler arriba para evitar disparos por
                // bClickLatched stale + cambio de hover.
                // Click sobre un NPC: setea el objetivo de movimiento y pathfindea (gateado por un click real)
```

### Línea 1642 en `Player_ProcessInput` — antes de `TargetX = (DWORD)dstX;`

```cpp
                    // 2026-05-07 BUG-FIX: dst grid coords del NPC, NO el animTick.
```

### Línea 1661 en `Player_ProcessInput` — antes de `*(unsigned char*)(ent + 0x2ed) = 1;`

```cpp
                // 2026-05-06: bClickEdge en vez de bHoverActive (mismo fix).
```

### Línea 1664 en `Player_ProcessInput` — antes de `int itemSlotIdx = (int)SelectedItem;`

```cpp
                // 2026-07-27 BUG-FIX: SelectedItem es índice del pool de items
                // del suelo (DAT_07e12840, stride 0x204), NO del pool de
                // personajes (DAT_07abf5d0, stride 0x394). El port anterior leía
                // el destino del pool equivocado → coords basura → el héroe
                // caminaba a cualquier lado. El tile del item = worldXY/100
                // (world = base+16/20).
```

### Línea 1673 en `Player_ProcessInput` — antes de `int dstX = (int)(*(float*)(itemEnt + 88) / 100.0f);`

```cpp
                // 2026-07-27 BUG-FIX: la posición world del item la escribe
                // CreateItem en ip+88/92 (no ip+16). Leer ip+16 daba (0,0) → el
                // héroe caminaba al origen del mundo (nada). El render lee la pos
                // en v1+16 = ip+72+16 = ip+88; el item-base (itemEnt) = ip, así
                // que la pos está en itemEnt+88/92.
```

### Línea 1698 en `Player_ProcessInput` — antes de `if (DAT_083a42c4 == 0 && !bClickEdge) goto end_tick_inc;`

```cpp
            // ── Ground click: ray cast → terrain check → pathfind ────────────
            // 2026-08-17 — REVERTIDO el gate one-shot de 2026-04-28.
            //
            // El gate era `if (!bHoverActive) goto end_tick_inc;`, y bHoverActive
            // es one-shot (lo cierra el latch s_clickCycleConsumed en la línea
            // ~1183). Eso convertía MANTENER el botón en un click único: el héroe
            // daba un paso y se plantaba.
            //
            // El original NO hace eso. MoveHero @ 0x004ACEF0:
            //     bVar32 = MouseLButtonPush != false;
            //     if (bVar32) MouseLButtonPush = false;      // consume el flanco
            //     bVar31 = MouseLButton != false || bVar32;  // ESTADO SOSTENIDO || flanco
            //     if (MouseLButton == false && !bVar32) { ...sale sin mover... }
            // bVar31 — lo que habilita el movimiento — es el estado en tiempo real
            // del botón O el flanco de bajada. Mantener el botón camina de forma
            // continua: es el comportamiento clásico del MU.
            //
            // El comentario del fix viejo decía "sin esto el hero seguía al mouse
            // continuamente sin click": seguir al mouse mientras el botón está
            // apretado ES lo correcto. El bug real era que DAT_083a42c4 quedaba
            // pegado en 1 tras soltar (de ahí el "sin click"); hoy WndProc lo
            // mantiene bien (WinMain.cpp:1182-1204), así que la causa ya no existe.
            //
            // La repetición la limita el debounce de la línea ~955
            // (DAT_00559bec <= DAT_07e11d28), igual que el original la limita con
            // MouseUpdateTimeMax <= MouseUpdateTime. Los gates de UI de abajo
            // (g_MouseOnWindow, s_clickStartedOnWindow) siguen intactos.
            //
            // 2026-08-17 (b): leer DAT_083a42c4 EN VIVO, no la copia bClickHeld
            // capturada en la línea ~1059. Las líneas ~1214-1216 limpian los flags
            // de click cuando el cursor pasa a estar sobre una ventana, y con la
            // copia vieja ese limpiado no tenía efecto hasta el frame siguiente:
            // manteniendo el botón y arrastrando el cursor sobre la UI, el ground
            // click seguía recalculando destino desde el píxel bajo el cursor y,
            // como la cámara sigue al héroe, el destino huía con ella → caminata
            // infinita. Con el estado en vivo el hold se corta en el acto, igual
            // que el original, que lee MouseLButton directo y no una copia.
```

### Línea 1736 en `Player_ProcessInput` — antes de `if (g_MouseOnWindow) goto end_tick_inc;`

```cpp
            // 2026-05-04: per IDA Player_InputTick:416,566 — block ground click
            // cuando el mouse está sobre cualquier panel abierto (MouseOnWindow=1). Sin
            // esto, clickear el botón [+] de stats o la X de cerrar del panel también
            // hacía caminar al jugador hacia esa posición de pantalla.
```

### Línea 1741 en `Player_ProcessInput` — antes de `if (s_clickStartedOnWindow) goto end_tick_inc;`

```cpp
            // IDA: el click al mundo NO cierra ventanas de NPC acá. Lo hace
            // SendMove (0x491C40) al mandar el movimiento: el personaje camina
            // y la ventana se cierra con su paquete (Combat.cpp,
            // SendMove_CloseWindows97k). MuEmu no rechaza el 0x10 con la
            // interfaz abierta (CGMoveRecv no la chequea).
            // 2026-05-05: También bloquear si el click se inició sobre window
            // (caso: user click skill cell, Chat_InputTick consume y resetea
            // DAT_07db870c → siguiente frame g_MouseOnWindow=0 pero el click
            // tail aún propagating como bHoverActive=true).
```

### Línea 1751 en `Player_ProcessInput` — antes de `{`

```cpp
            // 2026-05-05: hard gate — si la skill expanded list estuvo abierta
            // este frame O el frame anterior, ningún ground click vale.
            // Cubre el race entre Chat_InputTick reset y Player_InputTick check.
```

### Línea 1772 en `Player_ProcessInput` — antes de `extern void Map_InitRayCast(void);`

```cpp
                    // BUG-FIX 2026-04-29: reset closest-hit sentinel ANTES de
                    // cada scan. Sin esto, CollisionDetectLineToFace rechaza todos los hits
                    // si DAT_083a4120 (t_max) quedó stale de un frame previo.
```

### Línea 1783 en `Player_ProcessInput` — antes de `float pickWX = *(float*)&DAT_080ab288;`

```cpp
                        // BUG-FIX 2026-04-30: el "fix 2026-04-28" estaba MAL.
                        // En realidad DAT_080ab288/28c YA viene en grid coords
                        // (e.g. 218.0) — el picker (RenderTerrain) hace la
                        // conversión interna con _DAT_005524f0.  Dividir otra
                        // vez por 100 producía siempre gridX=2 gridY=0 (218/100
                        // → 2 truncado) y bloqueaba el movimiento porque
                        // pathfind iba siempre al mismo destino imposible.
                        //
                        // Evidencia del log: pickWX=218.0 (grid 218), no 21800.
                        // Cast directo a int.
```

### Línea 1871 en `Player_ProcessInput` — antes de `end_tick_inc:`

```cpp
    // 2026-09-02 FIX (hay que clickear varias veces para caminar): aca habia
    // un `else` que, cuando el gate de debounce bloqueaba, hacia
    //     MouseLButtonPush = 0; MouseLButton = 0;
    //
    // IDA Player_InputTick (0x4ACEF0 L586-588) NO borra nada en ese camino:
    //     if ( MouseUpdateTime < MouseUpdateTimeMax || byte_7E11DC0 )
    //         goto LABEL_390;            // == ++MouseUpdateTime; salir
    // Los dos flags solo se limpian en el anti-AFK de L607-611 (boton
    // sostenido 3600 s). O sea el click PENDIENTE sobrevive al bloqueo y lo
    // procesa el primer tick que pase el gate.
    //
    // Al borrarlos se perdia el click: con el boton sostenido, el primer tick
    // bloqueado mataba MouseLButton y el caminar continuo se cortaba; con un
    // click corto se perdia el pulso entero y habia que volver a clickear.
    // Medido en debug.log: 161 WM_LBUTTONDOWN -> solo 57 GroundClick.
```

### Línea 1894 en `Player_ProcessInput` — antes de `{`

```cpp
    // 2026-08-16: el port tenía DOS errores acá y por eso `HeroTile` era basura:
    //   1. Leía `DAT_05826e08` (**WorldTime**) en AMBOS ejes, no la posición del
    //      héroe. El comentario lo admitía ("simplified").
    //   2. Componía el índice invertido (`gy + gx*256` en vez de `gx + gy*256`).
    //
```

## `src/Game/Timer.cpp`

### Línea 41 en `Timer_UpdateFrameTiming` — antes de `if (WorldTimeEpochMs == 0) WorldTimeEpochMs = DVar2;`

```cpp
  // DESVIACION DELIBERADA (2026-09-08).  El binario hace
  // `fild qword [timeGetTime()]; fstp dword WorldTime`, o sea guarda los ms
  // DESDE EL ARRANQUE DE WINDOWS en un float de 4 bytes.  Pasadas ~13 horas de
  // uptime el ULP del float supera 1 ms y WorldTime deja de poder representar
  // cada milisegundo; con 34 dias encendido el ULP es de 256 ms, asi que
  // WorldTime se queda quieto ~6 frames y despues salta de golpe.  Eso se ve en
  // el reloj de Blood Castle (`sub_4BF2D0` imprime `WorldTime % 60`), y tambien
  // en el cursor, el agua y las UV animadas -- todos consumidores del mismo
  // global (ver la nota de `(int)WorldTime` que satura a los 24.8 dias).
  //
  // El cliente de referencia NO tiene el problema porque el DLL de inyeccion
  // reemplaza CalcFPS entero (`SetCompleteHook(0xE9, 0x0043FD70, ...)`) y ahi
  // hace `WorldTime = (float)clock()`, o sea ms desde que arranco el PROCESO.
  // Se adopta la misma base: contando desde el arranque del cliente el float
  // conserva precision de 1 ms durante 4.6 horas y de 2 ms hasta las 9.3.
```
