# Historial de comentarios: `src/Combat/`

Comentarios de desarrollo movidos desde `src/Combat/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Combat/Combat.cpp`

### Referencia a un archivo que ya no existe (línea 437)

```cpp
//   CharData_RecalcStats @ 0x0047e3c0  (ver UI.cpp)
```

### Línea 466 — antes de `extern "C" int g_bServerDivisionEnable;`

```cpp
// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 6475-7690 (1216 lines)
// Combat_SendMovePathPacket (Send_MovePacket), Combat_DispatchHeroSkillAttack (Attack), Combat_CheckArrowRequirement (CheckArrow),
// Combat_UseElfSkill (UseSkillElf stub), Action (Action big switch),
// TERRAIN_INDEX (Terrain_GetAttrDirect)
// =============================================================================
```

### Línea 577 en `Combat_SendMovePathPacket` — antes de `if (wpCount == 0)`

```cpp
    // IDA sigue aunque no haya camino (activa la ruta y cierra ventanas), pero
    // ahi solo se llama tras un PathFinding exitoso. Se conserva como resguardo:
    // un llamado sin camino reactivaba la ruta vieja y el heroe atravesaba
    // paredes (2026-09-17).
```

### Línea 587 en `Combat_SendMovePathPacket` — antes de `unsigned char pkt[16];`

```cpp
    // 2026-05-05 BUG-FIX: el packet de move tenía la nibble inversa y length
    // fijo. Per IDA decompile Combat_SendMovePathPacket + server CGMoveRecv:
    //   path[0] = (dir0 << 4) | (wpCount - 1)
    //   path[1..] cada byte packs 2 dirs: high=dir[2k+1], low=dir[2k+2]
    //   total length = 5 + ((wpCount >> 1) + 1) bytes
    // Antes mandábamos: path[0] = (wpCount << 4) | dir0  — server leía Dir=wpCount
    // y PathCount=dir0 — no path procesado — server ignoraba y char snapeaba.
```

### Línea 599 en `Combat_SendMovePathPacket` — antes de `pkt[3] = *(unsigned char*)(param_1 + 0x357); // path_wp_x[0] = start grid X`

```cpp
    // 2026-05-05 BUG-FIX: IDA decompile Combat_SendMovePathPacket muestra que pkt[3]/pkt[4]
    // son `entity[+0x357]` y `entity[+0x366]` = path_wp_x[0]/path_wp_y[0]
    // (= START de la path = current grid pos), NO entity[+0x306]/[+0x307]
    // (= target del último move server-confirmed). Server lee pkt[3]/[4] como
    // PathX[0] y walks PathX[1..PathCount-1] aplicando los dirs. Si pkt[3]/[4]
    // era el target, server simulaba walk DESDE target — char acababa en
    // posición incorrecta — server snap-back con GCTeleportSend.
```

### Línea 623 en `Combat_SendMovePathPacket` — antes de `BYTE dir = 0;`

```cpp
        // 2026-05-05 BUG-FIX: dir encoding debe matchear server's RoadPathTable
        // (Util.cpp:19): { (-1,-1), (0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0) }.
        // Antes el mapping estaba rotado +1 — server walk a dirección equivocada
        // — tiles bloqueadas — server respondía con 0x11 snap-back.
```

### Línea 658 en `Combat_SendMovePathPacket` — antes de `static const BYTE s_MoveKey[32] = {`

```cpp
    // BUG-FIX 2026-07-19 (DESCONEXIÓN AL MOVERSE): esto usaba
    // `Net_SendSmallPacket`, que es el path **C3** (Game_SceneUpdate.cpp:206):
    //   pkt[1] = serial++;            ← PISA el byte de TAMAÑO del C1
    //   buf[0] = 0xC3; ... encrypt;   ← re-enmarca como C3 cifrado
    // El paquete de movimiento es un **C1 plano** (`C1 len 10 X Y path…`), así
    // que salía con el tamaño destruido y envuelto como C3. El server lo
    // descifraba como C3, obtenía basura y cerraba la conexión (FD_CLOSE ~50ms
    // después de cada envío de movimiento). Además Net_SendSmallPacket aplica
    // su propio chain-XOR, con lo que se duplicaba el que hacíamos acá.
    //
    // Path correcto para C1 (igual que Pkt_Send en Game_EnterWorldTick):
    // chain-XOR y `send()` directo — el hook de send() aplica el MuEmu byte-XOR
    // automáticamente a los C1 planos (líneas "AUTO-ENCRYPT C1" del log, que
    // brillaban por su ausencia en los envíos de movimiento).
```

### Línea 694 — antes de `// Helpers: real names exposed via functions.h`

```cpp
// ──────────────────────────────────────────────────────────────────────────
// IDA: Attack @ 0x0049CBF0 — Attack(c)  [PORTED 2026-05-05]
//
// Tamaño binario: 62649 bytes (la función más grande del cliente).
// Decompile IDA: 10112 líneas con cientos de stack vars de obfuscation.
//
// PROP??SITO: Despachador de SKILL/ATTACK del HERO. Llamado desde MoveCharacter
// (per-frame del hero) cuando el usuario tiene un click derecho activo o cuando
// auto-attack está enabled. Lee la SKILL EQUIPADA en el slot activo
// (CharacterAttribute[Hero[913] + 87] = iType) y dispatcha por tipo:
//
//   iType <  30  — ataque/skill básico (Item_Equip / SkillElf / fall-through)
//   iType == 16  — SkillTeleport (party)  — packet 0x19 [iType][TgtH][TgtL]
//   iType == 5,6,7,9: skills DK con animación (case 5: Twister)
//   iType == 51  — Twister/triple shot (DK swing)
//   iType == 52  — Cyclone/whirlwind     — packet 0x1E [iType][gridX][gridY][angle][0][0]
//   iType == 26..28 — Heal/Buff (Elf)    — UseSkillElf
//   iType == 47  — Death Stab (DK)       — gated CheckAttack()
//   iType == 48  — Mount-only attack
//
// PACKET LAYOUTS (verified vs IDA disasm):
//
// Packet 0x19 (PMSG_MAGIC_ATTACK / "Skill use targeted"):
//   [0xC1][0x06][0x19][skillID:1][TargetID_hi:1][TargetID_lo:1]
//   Server→client damage broadcast lleva opcode 0x18 con [TgtId][HP%][Dmg].
//
// Packet 0x1E (PMSG_AOE_SKILL / "Skill use directional"):
//   [0xC1][0x09][0x1E][skillID:1][gridX:1][gridY:1][angle:1][unk:1][unk:1]
//   Para Twister/Cyclone, el server hit detecta entities en cone+radio.
//   "angle" = angle_deg * 0.71111113 (= 256/360, dirección packed en byte).
//
// Packet 0x1C (PMSG_TELEPORT):
//   [0xC1][0x05][0x1C][skillID][TargetX:1][TargetY:1]
//
// Packet 0x26 (PMSG_USE_INVITEM):
//   [0xC1][0x04][0x26][slot+12:1][reserved:1]
//   skill que requiere mana: usa scroll de inventario antes de cast.
//
// Packet 0xB0 (PMSG_PARTYRECALL): variante usada en case iType==15.
//
// PACKET DE ATAQUE BÁSICO (sword, fist):
//   NOT in this function. Está en Action() @ 0x0048D640. Esta función SOLO
//   maneja skills (iType >= 1). La detección de hit melee directa se envía
//   desde Action vía opcode 0x11 con (TargetX, TargetY, heading).
//
// DAMAGE RECEIVE PACKETS (server→client, en Net_Process.cpp):
//   0x11 — spawn de monsters/players con HP
//   0x14 — kill confirm + EXP
//   0x16 — muerte + ragdoll spawn (target_dead_flag=1, anim=6)
//   0x17 — HP update (DAT_07d76690 = HP/MaxHP)
//   0x18 — damage display (entity recibió N de daño)
//   0x19 — skill broadcast (otra entidad usó skill ID X sobre target Y)
//   0x1A — entity attack target (anim 0x5A play)
//
// FORMA REAL DEL CFG EN IDA (verificada 2026-09-01 contra el decompile y el
// disassembly; nuestro port es una reconstruccion POR ID DE SKILL, no una copia
// estructural, y conviene tenerlo presente antes de tocar nada):
//
//   L1236..L1344  prologo comun (EditFlag, alpha, botones, muerto/SafeZone,
//                 iType, Attacking, MouseOnWindow, gate de animacion)
//   L1346..L1484  if ((c+444 & 7) != 0) { CheckTarget(c);
//                    if (CheckWall(heroGX, heroGY, TargetX, TargetY))
//                       for (i = 0; i <= 68; i += 68) {          // LAS DOS MANOS
//                          clase 1/3 -> Item_Equip(c, CharacterMachine+536+i)
//                          clase 2   -> SkillElf  (c, CharacterMachine+536+i)
//                          si devuelve != 0 -> LABEL_146 (return)
//                       } }
//   L1485..L2893  if ((c+444 & 7) == 2)  { rama ELF completa }
//                 (0x0049D1EB: `mov cl,[ebp+1BCh]; and cl,7; cmp cl,2;
//                  jnz 0x0049F195` — o sea el chequeo de mana/scroll y el
//                  dispatch de skills bajos de este bloque son SOLO de Elf)
//   L2894 LABEL_322  if (v183 == 1 || v183 == 3) { rama DK/MG }
//                    switch (iType) { 41, 42, 48, 55, 56, default }
//   L6246         HIBYTE(v1025) = CheckTarget(c);   // <- CheckTarget SI devuelve bool
//   L6247         switch (iType) { 10, 14, 16, ...,
//                                  18/19/20/21/22/23/55/56 -> return;  default }
//   L7960         switch anidado (LABEL_1354): 5, 8, 9, 12, default
//
// El port conserva los gates de clase y el CheckWall previo al bucle de manos,
// pero representa los labels del CFG como helpers con retornos AF_RETURN /
// AF_LABEL_322 en lugar de reproducir los gotos literales.
//
// LIMITACIONES de este port:
//
//   - Anti-tamper hash table ops (sub_4041E0/sub_403F80/sub_404280/sub_404370/
//     sub_404400) están skipped per project policy. En el binario original son
//     refcount + XOR encryption sobre CharacterMachine.
//   - Las XOR keys de packet body (los 32 bytes v998..v1023) están skipped:
//     usamos Net_SendSmallPacket() que ya las aplica via MuEmu::EncryptSend.
//   - Las strings webzen anti-cheat (aWebzen_17..aWebzen_31 = "WEBZEN") son
//     cliente-side anti-mod check; siempre pasan en builds limpios — no port.
//   - Los 65 send-blocks inline (cada uno ~130 líneas C0/C1/C3/C4 wrapping)
//     se reducen a una sola call Net_SendSmallPacket().
//
// CALLER: MoveCharacter — llama esta función cuando el hero
// tiene flag de attack activo. También llamada desde UseSkillWarrior y
// UseSkillElf como fallback continuation.
```

### Línea 800 — antes de `// 0049CCAA..0049CCF1.  The original keeps these independently of the UI`

```cpp
// 2026-09-01: aca vivian dos statics locales (`g_dwLatestMagicTick_Attack` y
// `g_dwLatestTeleportRequest_Attack`) que reemplazaban a los globals del binario
// porque se creia que sus direcciones aliaseaban timers de UI/NPC.  Es falso:
//   g_dwLatestMagicTick (ya usado por UseSkillWarrior/Wizard)
//   dword_7E11DC8 / DC4  = DAT_07e11dc8 / DAT_07e11dc4 — los escribe
//                          ReceiveTeleport (0x428210, xref 0x428EBF) y los leen
//                          Attack (0x4AB5E7) y CheckGate (0x4AC6DE).
// Con los statics el cooldown de teleport de Attack no compartia estado con el
// que arma el servidor, asi que los dos gates corrian por separado.
```

### Línea 920 — antes de `static BYTE Combat_GetDestValue97k(int xPos, int /*yPos*/, int xDst, int /*yDst*/)`

```cpp
// Byte `dis` del C3:1E (cases 55 y 56, y Triple Shot).
//
// 2026-09-02: verificado a nivel de INSTRUCCION y unificado con el sitio
// gemelo de UseSkillWarrior.  El binario calcula los DOS nibbles a partir del
// MISMO delta X, con +8 arriba y -8 abajo.  No es un artefacto de Hex-Rays:
// en 0x00486070 (UseSkillWarrior) la secuencia es literalmente
//     8A 44 24 7C   mov al, [esp+7Ch]      ; TargetX
//     8A 4C 24 10   mov cl, [esp+10h]      ; heroGridX
//     2A C1         sub al, cl             ; UN solo delta
//     8A C8         mov cl, al
//     2C 08         sub al, 8
//     80 C1 08      add cl, 8
//     24 0F         and al, 0Fh            ; nibble bajo  = (delta - 8) & 0xF
//     C0 E1 04      shl cl, 4              ; nibble alto  = (delta + 8) << 4
//     0A C1         or  al, cl
// y Hex-Rays emite la misma expresion en los tres sitios (Attack L4914 y
// L5533, UseSkillWarrior).  O sea es un bug del 0.97k original, no del port.
//
// El DLL de inyeccion lo corrige: `GetDestValue` (Source/Client/Main/Util.cpp:323)
// usa dx para el nibble alto y dy para el bajo, los dos con +8 y clampeados a
// [-8, 7] -- igual que el helper del 5.2 (source/wsclientinline.h:615).  Pero el
// DLL reemplaza el envio ENTERO por su SendRequestMagicContinue, asi que eso es
// una mejora suya, no evidencia sobre el binario.  Por politica del proyecto
// (IDA manda; las mejoras del DLL van al final) se deja la forma original.
//
// Da igual funcionalmente: MuEmu **no lee `dis`**.  CGDurationSkillAttackRecv
// (GameServer/SkillManager.cpp:2047) pasa solo x, y, dir, angle e index[].
```

### Línea 1351 en `Attack_Label1585_97k` — antes de `if (Teleport || DAT_07e11dc4 || (GetTickCount() - DAT_07e11dc8) < 3000)`

```cpp
        // IDA L9303: Teleport || dword_7E11DC4 || GetTickCount() - dword_7E11DC8 < 3000.
        // Los tres globals existen en nuestro arbol y ya los escribe el handler
        // 0x1C de Net_Process (ReceiveTeleport), asi que el cooldown queda
        // compartido igual que en el binario.
        // `Teleport` de IDA es 0x05826D14 (Teleport), el mismo flag que
        // limpian ReceiveTeleport, el 0x19/0x0F, ReceiveRevival y CheckGate.
        // El port usaba DAT_05826d04, otro global (lo usan ReceiveLogOut y
        // UI_InGameMenu): el flag del skill nunca se limpiaba donde debia.
```

### Línea 1540 en `Combat_DispatchHeroSkillAttack` — antes de `if (entity[765] != 0) return;`

```cpp
    // 4) Death / SafeZone gate (IDA Attack L1280) —
    //      if (c[765] || c[846] && (World < 11 || World > 16)) return;
    // +765 (0x2FD) = dead_flag real (lo setea ReceiveDie).
    // +846 (0x34E) = **SafeZone**, NO "mount-only": vale
    //      TerrainWall[Terrain_Load(x,y)] & 1.  La etiqueta vieja venia de la
    //      tabla de offsets de CLAUDE.md, que estaba mal (ver la entrada
    //      "+0x34E es SafeZone, no dead_flag", 2026-08-10).  La logica ya era
    //      correcta; solo el comentario mentia.
    // O sea: muerto, o parado en zona segura fuera de los mapas 11..16.
```

### Línea 1570 en `Combat_DispatchHeroSkillAttack` — antes de `if (DAT_07e11e18 != 0 && (int)World != 6) {  // IDA: m_bAutoAttack, World`

```cpp
    // 7) 0049CBF0 escribe Attacking desde el estado de movimiento de la entidad. Ése es
    // el estado que consume el gate de auto-ataque de la invocación siguiente, arriba.
    // 2026-09-01 FIX — global partido en dos.  Esto escribia `DAT_07e11984`,
    // que en globals.h es el *debounce de la flecha arriba del chat* (lo escribe
    // Chat_InputTick con 0/1).  El `Attacking` de IDA vive en **0x00559C58**:
    // verificado con ida_xrefs_to — lo escriben InitGame L38 (=-1),
    // Player_InputTick L942 (=1) y este Attack (=2/-1), y lo leen el gate de
    // auto-ataque de arriba y Player_InputTick L599.  En nuestro arbol esa
    // direccion es `Attacking`, que Mouse_Hover ya usa con esa semantica.
    // Con el global equivocado el gate `Attacking == 2` no se cumplia nunca y
    // la continuacion de auto-ataque quedaba muerta.
```

### Línea 1591 en `Combat_DispatchHeroSkillAttack` — antes de `if (g_MouseOnWindow) return;             // IDA L1330: MouseOnWindow (0x07D78094)`

```cpp
    // 8) MouseOnWindow (IDA Attack L1330) — si el cursor esta sobre una ventana
    // de UI, no se ataca.  2026-08-15: estaba diferido ("no trackeamos ese
    // global"), pero SI existe: `g_MouseOnWindow`, que puebla
    // `MouseOnWindow_Update` en Player_InputTick.cpp (y al que el widget de chat
    // le pasa su latch `g_ChatLB_MouseOnWindow`).  Sin este gate, click derecho
    // sobre el inventario / chat / paneles disparaba el skill igual.
```

### Línea 1826 — antes de `// IDA: FUN_0048A180 @ 0x0048A180 — UseSkillElf(c, o).`

```cpp
// ──────────────────────────────────────────────────────────────────────────
// IDA: FUN_0048d640 @ 0x0048D640 — Action(DWORD c, DWORD o)
//
// IDA companion: raw/0048D640_Action.c — 2587 lines, 17558 bytes.
//
// Despachador de acciones del entity local: ejecuta la acción en cola en
// `*(c+749)` cuando el pathfind ha terminado o no hay distancia que recorrer.
// Out queue values (1-based, decoded as `*(c+749) - 1`):
//   1 — pickup ground item   (case 0)  — packet 0x22 (ground item request)
//   2 — equip / NPC interact (case 1)  — packet 0xa0 (talk-to-npc)
//   3 — attack target        (case 2)  — packet 0x10 (move) + 0x15 (attack)
//   4 — walk-to-location     (case 3)  — packet 0x10 (final position)
//   5 — cast skill           (case 4)  — invokes UseSkillWarrior/UseSkillElf
//
// Anti-tamper skipped: hash table refcount ops alrededor de CharacterMachine
// reads (sub_4041E0/sub_403F80/sub_404280/sub_404330/sub_404370/sub_404400),
// XOR encryption pass sobre CharacterMachine, phantom XOR-key local var
// bloques de re-init (32 bytes repartidos antes/después de cada loop de XOR encadenado), y
// PACKET_DECRYPT/PACKET_ENCRYPT calls obfuscadas alrededor de g_byPacketSerialSend.
//
// Wire packets:
//  - El binario original arma cada packet a mano: chained-XOR los bytes,
//    aplica CSimpleModulus (sub_53CC30) para encrypt, prefija C3/C4 framing,
//    y manda via send() con WSAEWOULDBLOCK queue. En nuestro port usamos el
//    helper Net_SendSmallPacket() que hace exactamente lo mismo + serial stomp.
//
// Notas para la implementación (status 2026-05-05):
//  - Caso 2 (attack): IMPLEMENTADO — calcula distancia al target, llama
//    SetPlayerAttack(c) para la animación local, y manda packet 0x15 con el
//    target ID (entity index). Si fuera de rango, intenta pathfind.
//  - Caso 4 (skill): IMPLEMENTADO parcialmente — dispatch sobre skill type
//    desde DAT_07d78098 / DAT_07d7809c, llama UseSkillWarrior / UseSkillElf
//    para los casos confirmados (47=warrior melee skill, 19-23/26-28/43/49/56=
//    elf magic). Pathfind si fuera de rango.
//  - Cases 0..3 retain their own native Action paths; the combat dispatch is
//    case 4 (the fifth 1-based queue action).
//
// Param semantics (per IDA):
//   c = CharacterMachine ptr (XOR-encoded char attribute buffer, ~0x584 bytes)
//       offsets accessed:
//         +444  (LOWORD) char class flags (bit 7 = MG/2nd-class)
//         +747  (BYTE)   weapon type 9 = bow
//         +748  (BYTE)   flag de camino completado (poner en 1 para empezar a caminar)
//         +749  (BYTE)   queued action id (1-based; 0 = no action)
//         +757  (BYTE)   "attack pending" flag
//         +784  (WORD)   last attack target id
//         +788..+800     última posición de impacto (floats xyz de CharactersClient)
//         +846  (BYTE)   "force-walk" flag (suppresses range check)
//         +852..+855     pathfind 4-byte ctx (passed to PathFinding2)
//         +854  (BYTE)   v309/v307 location lock flag (cleared after teleport)
//         +904  (DWORD)  hero grid X (encrypted)
//         +908  (DWORD)  hero grid Y (encrypted)
//   o = OBJECT ptr (= player entity in CharactersClient[heroIndex])
//       offsets accessed:
//         +2    (WORD)   model type (390=hero player, !=390 NPC/mob)
//         +16   (float)  world X (cm)
//         +20   (float)  world Y (cm)
//         +36   (float)  facing angle (0..360 degrees)
//         +261  (BYTE)   anim_state_prev (se usa para gatear la cadena de ataque)
//
// Helpers used:
//   SetPlayerAttack         SetPlayerAttack — local attack anim dispatcher
//   PathFinding2            Path_FindRoute — pathfind helper (returns 0/1)
//   Movement_Tick           CreateAngle — atan2-based facing-toward (CreateAngle)
//   CheckWall               Path_IsLineClear — line-of-sight tile check (PathRange_Check)
//   UseSkillWarrior         Combat_UseWarriorSkill
//   UseSkillElf             Combat_UseElfSkill
//   SetPlayerStop           SetPlayerStop — sets idle anim
//   PlayBuffer              PlayBuffer — sound effect by id
//   SetAction               SetAction — set entity action
//   sub_4889D0              FUN_004889d0 — skill-attack-finalize helper
//   Net_SendSmallPacket     project helper (Net.h) — does C3 wrap + chain XOR +
//                                                    CSimpleModulus + serial
// ──────────────────────────────────────────────────────────────────────────
```

### Línea 1933 en `Combat_UseElfSkill` — antes de `const DWORD now = GetTickCount();`

```cpp
    // 2026-09-02: aca habia un `if (targetKey == 0xFFFF) return;` inventado.
    // UseSkillElf (0x0048A180) no lo tiene: manda el paquete con la key tal cual
    // la lee y despues SIEMPRE anima (SetPlayerMagic o SetPlayerAttack).  Con el
    // guard, un slot en estado raro se comia el skill Y la animacion.
```

### Línea 1971 — antes de `// Aliases to match IDA companion variable names`

```cpp
// Declaraciones adelantadas de los helpers que usa Action — casi todos ya están declarados en
// functions.h con los mismos prototipos; los re-declaramos localmente para evitar
// implicit-decl warnings if a particular helper hasn't been wired up yet.
//
// IDA: FUN_004889D0 is ported as Combat_UseWizardSkill in
// stubs_game.cpp y es el emisor de skill directo que usa Action en el case 4.
```

### Línea 1982 — antes de `#define ACTION_CHARS_CLIENT  ((char*)(uintptr_t)DAT_07abf5d0)`

```cpp
// 2026-05-06 BUG-FIX MAYÚSCULO: usar DAT_07abf5d0 directamente, NO el alias
// `g_EntityBase` que está declarado nullptr en stubs.cpp:73 y nunca se
// asigna. ACTION_CHARS_CLIENT + 916*targetIdx + 16 = NULL+0x738 = AV
// (user reportó crash apenas entrar al mundo addr=0x55D1D6 param1=0x738
// 2026-05-06).
```

### Línea 1991 — antes de `void __cdecl Action(DWORD c, DWORD o)`

```cpp
// Como en IDA (0x0048D640), Action solo LEE la cola c+749; la reescribe
// Player_InputTick en cada click.  (2026-09-18: se sacaron los cinco clears y
// el tick secundario que los obligaba.)
// IDA: Action (0x0048D640)
```

### Línea 2103 en `Action` — antes de `const unsigned short npcKey = *(unsigned short*)(npcEnt + 0x1DC);`

```cpp
        // IDA LABEL_297 (L952-1189): PMSG_NPC_TALK_RECV
        // (GameServer/NpcTalk.h:10) = [C1][05][30][index[2] big-endian].
        // 2026-09-02: se removio un `if (npcKey != 0xFFFF)` inventado -- el
        // binario manda la key tal cual la lee.
```

### Línea 2134 en `Action` — antes de `const char* const CM = (const char*)(uintptr_t)DAT_07cf1ffc;`

```cpp
        // IDA Action 0x0048D640 L1194-1212 — alcance segun el arma equipada.
        //   v11 = *(__int16 *)(CharacterMachine + 536);   // wear slot 0 (mano izq)
        //   v12 = *(__int16 *)(CharacterMachine + 604);   // wear slot 1 (mano der)
        // OJO: el original lee el global CharacterMachine, NO `c`.  El port leia
        // `c + 536` / `c + 604`, que en la entidad (stride 916) son campos sin
        // relacion, asi que Range nunca salia del default 1.8 y el arco pegaba
        // solo cuerpo a cuerpo.  Los ids son TIPOS de item (sin el +400 del
        // modelo): 136-142 arcos, 128-134 ballestas, 145 el par arco/ballesta.
```

### Línea 2305 en `Action` — antes de `// ── 1. Distance gate ───────────────────────────────────────────────────────────────────`

```cpp
        // 2026-05-08: port FIEL completo de IDA Action.c L1417-2243.
        //
        // 1) Gate de distancia: toma el eje mayor de abs(heroGrid - target). Si
        //    es > 1 tile, el héroe todavía no llegó — retorna y espera.
        // 2) World+tile-id (dword_7DB8708) dispatch - marca v307/v309 y,
        //    opcionalmente, pisa el facing del heroe con flt_7E118E4 antes de
        //    mandar el paquete de movimiento.  Esto NO son teleports: son las
        //    acciones sobre el mobiliario del mundo (sillas, bancos, barandas).
        //    v307 = SIT (sentarse) . v309 = POSE (apoyarse en la pared).
        // 3) Caso especial Noria (World 3, tile 38): HEALING -- la pose de
        //    flotar sobre los orbes.  Paquete propio (accion 110) y
        //    SetAction(137/138).  Retorna temprano.
        // 4) Default: send move packet [0xC1][len][0x10][TgtX][TgtY][heading]
        //    [wpCount][wpX[]][wpY[]] via legacy helper.
        // 5) Despues del paquete de movimiento:
        //    - Si v309: paquete [0xC1][0x06][0x18][0x01][heading][109] con
        //      SetAction(139/140) - POSE.
        //    - Si v307: paquete [0xC1][0x06][0x18][0x01][heading][108] con
        //      SetAction(133/135) - SIT.
        //    En los tres casos el payload lleva el octante del facing:
        //      dir = (int)((Angle[2] + 22.5) * (8/360) + 1) & 7
```

### Línea 2330 en `Action` — antes de `int TargetX_v = (int)TargetX;`

```cpp
        // 2026-09-04 FIX: el comentario anterior decia "en nuestro build TargetX/Y
        // no son globals" y leia `o + 0x306/0x307`.  Es FALSO: TargetX/TargetY son
        // 0x07E016C0 / 0x07E016C4 (= TargetX/c4), los mismos que escriben
        // `CheckTarget` y el bloque de SelectedOperate de Player_InputTick.
        // El +0x306/0x307 lo setea SOLO el click al suelo, asi que para una accion
        // sobre mobiliario tenia valores viejos y el gate cortaba con `return`:
        // el cursor cambiaba, el paquete de movimiento salia, pero la accion
        // (sentarse / apoyarse / flotar) no se ejecutaba nunca.
        //
        // IDA elige el eje de MAYOR delta y gatea sobre ese:
        //   if ( abs(heroX - TargetX) <= abs(heroY - TargetY) ) { v = heroY; t = TargetY; }
        //   else                                               { v = heroX; t = TargetX; }
        //   if ( abs(v - t) > 1 ) return;
```

### Línea 2380 en `Action` — antes de `if (TileSub == 8) {                                 v307 = true; }`

```cpp
            // AMBIGUEDAD RESUELTA (2026-09-04).  El decompile cierra el bloque
            // de mundos con
            //     if (World != 3) { if (World == 7) {...}
            //                       if (World != 8 || tile != 78) { LABEL_367: ... } }
            //     if (tile == 8)  goto LABEL_392;   // v307
            //     if (tile != 38) goto LABEL_367;
            //     <SetAction 137/138 + accion 110>
            // o sea la cola compartida se alcanza con World == 3.  El port
            // anterior la habia colgado de World 8 llamandola "entrada a la
            // cueva de Lost Tower"; no lo es.  El source de MU 5.2 lo confirma
            // termino por termino (ZzzInterface.cpp, MOVEMENT_OPERATE):
            //     WD_3NORIA:  case 8: Sit;  case 38: Healing + facing
            // y el "Healing" del 0.97k es la pose de flotar sobre los orbes de
            // Noria.
```

### Línea 2562 en `Action` — antes de `} else {`

```cpp
                // 2026-09-02: aca habia un `else if (Path_FindRoute(...))` que
                // hacia caminar al heroe cuando CheckWall fallaba.  IDA
                // (LABEL_184) no hace nada en ese caso: si la linea de vista
                // esta cortada, el skill simplemente no sale y la accion queda
                // encolada hasta el proximo click.
```

### Línea 2581 en `Action` — antes de `} else {`

```cpp
                // 2026-09-02: aca habia un `else if (Path_FindRoute(...))` que
                // hacia caminar al heroe cuando CheckWall fallaba.  IDA
                // (LABEL_184) no hace nada en ese caso: si la linea de vista
                // esta cortada, el skill simplemente no sale y la accion queda
                // encolada hasta el proximo click.
```

## `src/Combat/Combat_AttackEffect.cpp`

### Referencia a un archivo que ya no existe (línea 99)

```cpp
La version parcial de stubs_misc2.cpp tenia el mismo guard.
```

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Combat_AttackEffect.cpp
//
// AttackEffect (IDA 0x00445230) -- los efectos visuales del ataque de los
// MONSTRUOS.  Movida desde src/stubs_IDA_ports.cpp el 2026-09-27: vivia dentro
// de un `#if defined(IDA_PORT_00445230)` en el archivo-archivo de ports de IDA,
// pero esa macro SI esta definida, asi que era codigo vivo escondido en un
// archivo que por convencion "no se toca".
//
// El selector del switch es `Owner+747`, el TIPO DE MONSTRUO de Monster.txt.
// Para el heroe ese byte vale 0xFF, asi que no matchea ningun case: la funcion
// es un no-op para el jugador POR DISENO del binario.  Los efectos de los
// skills del jugador viven en los tres switches de MoveCharacter.
//
// Los `AE_*` de abajo son shims que neutralizan el ruido anti-tamper del
// decompile (hash-table, CErrorReport, operator_new) sin tocar la logica.
```

### Línea 22 — antes de `extern "C" void DbgLogPublic(const char* msg);   // probe AEDBG (temporal)`

```cpp
// === AttackEffect (0x00445230) — movida desde stubs_IDA_ports.cpp (2026-09-27) ===
// Estaba gateada por IDA_PORT_00445230, que esta definida: el gate era ruido.
```

### Línea 26 — antes de `namespace CErrorReport { inline void Write(DWORD, const char*) {} }`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// Shims de compatibilidad para el port crudo de AttackEffect (2026-08-16).
//
// El decompile de IDA trae el ruido anti-tamper tal cual (hash-table +
// CErrorReport + operator_new/delete alrededor de CADA lectura del byte de
// skill en Owner+770). Por policy del proyecto ese ruido no se ejecuta.
//
// Camino real en nuestro build: g_HashTableCtx tiene capacity=1 con el slot 0
// como centinela (key=0), asi que el primer `memcmp(&v245 /*0*/, keys[0] /*0*/)`
// da match, rompe el while y cae directo al `LABEL_*: Ownerx = *vN;` que lee el
// byte EN CLARO — que es exactamente lo que queremos (todo el resto del port
// escribe/lee Owner+770 sin encriptar). El segundo bloque (re-encrypt) no entra
// al while por la misma razon y no toca nada.
//
// Estos shims neutralizan las llamadas para que compile sin editar los ~60
// bloques a mano (menos riesgo de romper la logica de gameplay al reescribir).
// Se hace #undef de todo justo despues del #endif de esta funcion.
// ─────────────────────────────────────────────────────────────────────────────
```

### Línea 1900 en `AttackEffect` — antes de `TargetPosition[0] = *(float *)(Owner + 16);`

```cpp
            // 2026-09-04 -- DESVIACION DOCUMENTADA (bug del binario original).
            // Este case (Alquamos, MonsterID 69) usa `TargetPosition` y `v246`
            // SIN inicializarlos: son locales del frame que en IDA arrastran lo
            // que dejo un case anterior.  En nuestro build el CRT de Debug los
            // llena con 0xCCCCCCCC (= -1.07e8 como float), asi que la mitad de
            // los joints nacia en una coordenada absurda y sus lineas salian
            // disparadas al lado contrario del personaje.
            // Se anclan al propio monstruo, que es lo unico coherente con el
            // resto del case: `v213` ya es Owner+16 y el segundo CreateJoint
            // usa el mismo SubType 7, cuyo re-anclado de LABEL_182 deja la
            // Position pegada a la TargetPosition.
```

## `src/Combat/Combat_AttackStage.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 320 — antes de `bool __cdecl AttackStage(DWORD c, DWORD o)`

```cpp
// IDA: FUN_00451f30 @ 0x00451F30 (~52 lines) — death particle burst (20 dust particles)
// When entity is dead (anim==6) and animation frame is in range [_DAT_00552658, _DAT_00552830),
// spawn 20 dust particles (type 0x4c5) at random offsets (-32..+31) from entity position.
// Sets entity light to (1.0, 1.0, 1.0) before each particle spawn.
// Combat_SpawnDeathDustParticles (IDA-activated, was Ghidra stub)
// 00448930 AttackStage — direct IDA switch (raw/00448930_AttackStage.c).
// The older AttackStage_legacy_mismatched above is intentionally not called.
```

### Línea 365 en `AttackStage` — antes de `else if((*(float*)(o+264)>=1.0f && type==390 && *(BYTE*)(o+261)==62) || (*(float*)(o+264)>`

```cpp
    // -- PENDIENTE: grupo de skills de magia del DLL (mejora, NO esta en IDA) --
    //
    // El `else if` de abajo es el `default:` literal de IDA (0x00448930 L356-364):
    // pone `c+757 = 15` -- o sea deja que el skill dispare -- cuando el frame de
    // la animacion llega a 5.0 con la accion en 0x22..0x5B.
    //
    // PROBLEMA MEDIDO (2026-09-03, sonda ANIMSPD): `SetAttackSpeed` (0x00443E70)
    // le da a las acciones de casteo (82-85) una PlaySpeed de
    //     AttackSpeed * 0.004 + 0.29
    // Con el AttackSpeed de este server eso da 3.97 frames por tick y la accion
    // tiene 6 frames, asi que el frame tras avanzar va 3.97 -> 0.97 -> 3.97 ...
    // y NUNCA cae en [5, 6).  La condicion no se cumple, `c+757` no llega a 15 y
    // el efecto del skill no se crea; ademas cada eco `0x1E` del server lo
    // resetea a 1.  Sintoma: manteniendo el click derecho no aparece animacion ni
    // efecto hasta soltar, y sale una sola vez.
    //
    // LO QUE HACE EL DLL (Source/Client/Main/Patchs.cpp, CPatchs::AttackStage,
    // enganchado con SetCompleteHook(0xE9, 0x00448930)): agrega un grupo de cases
    // que el binario NO tiene, y que fuerza el disparo sin esperar el frame 5.0:
    //
    //     case SKILL_POISON: case SKILL_METEORITE: case SKILL_LIGHTNING:
    //     case SKILL_FIRE_BALL: case SKILL_FLAME: case SKILL_ICE:
    //     case SKILL_TWISTER: case SKILL_EVIL_SPIRIT: case SKILL_POWER_WAVE:
    //     case SKILL_AQUA_BEAM: case SKILL_BLAST: case SKILL_INFERNO:
    //     case SKILL_ENERGY_BALL:
    //         *(BYTE*)(c + 0x2F5) = 15;   // c->AttackTime = 15
    //         break;
    //
    // Para implementarlo aca alcanza con un `else if` sobre esos 13 ids ANTES
    // del default, poniendo `*(BYTE*)(c+757) = 15`.  Queda como mejora del DLL
    // pendiente de decision (politica del proyecto: IDA manda, las mejoras del
    // DLL van al final).
    //
    // Dato del usuario para tener en cuenta al implementarlo: en versiones
    // viejas de MU este mismo problema de velocidad de ataque se evita usando
    // MONTURA, que cambia el set de acciones del casteo (y por lo tanto su
    // cuenta de frames).  Conviene verificar el caso montado antes de dar el
    // fix por completo.
```

## `src/Combat/Combat_LegacyAnimation.cpp`

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

### Línea 51 — antes de `extern DWORD CharacterAttribute_var;     // alias - already in our globals`

```cpp
// IDA: SetAttackSpeed (0x00443E70)
// Real logic (after anti-tamper hash table blocks):
//   1. Reads CharacterAttribute->MagicDamageMax and AttackDamageMinRight
//   2. Computes animation speed: fVar2 = AttackDamageMinRight * _DAT_005524bc
//      fStack_8 = MagicDamageMax * _DAT_005524bc, local_18 = MagicDamageMax * _DAT_005528e0
// SetAttackSpeed @ 0x00443E70 — set player animation speeds based on
// CharacterAttribute stats. Port FIEL desde IDA decompile (2026-05-02).
//
// Reads CharacterAttribute[+0x38] (AttackSpeed) and [+0x44] (MagicSpeed),
// computes scale factors, then writes per-animation speed floats into the
// player model's animation table (Models[Player=390].Data[+0x30]).
//
// Anti-tamper hash table operations (sub_403F80/sub_4041E0/sub_404370/etc
// wrapping CharacterMachine encrypt/decrypt) are skipped per project policy.
//
// Note: IDA decompile shows v33 and v38 as uninitialized stack locals used
// for FIST/SWORD/RIDE-attack speeds. The disasm doesn't show explicit
// assignments visible in hex-rays output — treating as 0 (stack default).
// For attack-speed stat scaling on melee, this means baseline speeds are
// used. Magic skills (which use v34 = AttackSpeed*0.004 and v35/v39 from
// MagicSpeed) DO scale per-stat correctly.
//
// Animation table layout (from Models[390]+0x30 base, P):
//   P+548   FIST          v33 + 0.6
//   P+560..880 (stride 16, at i-12)  SWORD/RIDE attacks v33 + 0.25
//   P+900,916 SWORD1/2 magic         v33 + 0.3
//   P+932   SWORD3                   v33 + 0.27
//   P+948   SWORD4                   v33 + 0.3
//   P+964,980 SWORD5/WHEEL           v33 + 0.24
//   P+996   FURY_STRIKE              0.38 (constant 0x3EC28F5C)
//   P+1012  VITALITY                 0.34 (constant 0x3EAE147B)
//   P+1028  RIDER                    v33 + 0.3
//   P+1044  RIDER_FLY                v33 + 0.3
//   P+1060  SPEAR                    v33 + 0.3
//   P+1076  ONE_TO_ONE               v33 + 0.3
//   P+736..784 (stride 16, at i-12)  BOW attacks  v35 = MagicSpeed*0.002
//   P+864..880 (stride 16)           RIDE BOW     v35
//   P+1300  TWO_HAND_SWORD_TWO       v33 + 0.25
//   P+1312..1360 (stride 16)         HAND/WEAPON  v34 + 0.29
//   P+1380  ELF1                     v38 + 0.25
//   P+1396  TELEPORT                 v34 + 0.3
//   P+1412  FLASH                    v34 + 0.4
//   P+1428  INFERNO                  v34 + 0.6
//   P+1444  HELL                     v34 + 0.5
//   P+1460  RIDE_SKILL               v34 + 0.3
```

### Línea 111 en `SetAttackSpeed` — antes de `float v33 = v34;   // AttackSpeed * 0.004`

```cpp
    // 2026-08-24 FIX (animaciones largas terminaban tarde): aca decia
    //     float v33 = 0.0f;  float v38 = 0.0f;
    // con el comentario "uninitialized in IDA decompile", asi que TODA la tabla
    // de velocidades quedaba en su valor base (0.6, 0.25, 0.30...) sin escalar
    // por los stats del personaje. Con AttackSpeed alto la diferencia es del
    // orden del doble: el original terminaba la animacion y el nuestro seguia.
    //
    // Que v33/v38 aparezcan sin inicializar es un artefacto de Hex-Rays, no del
    // binario: `v34` (= AttackSpeed*0.004) y `v39` (= MagicSpeed*0.004) SI se
    // calculan y despues no se usan para nada, mientras v33/v38 se usan sin
    // origen. Los slots intermedios (v34/v35/v36) estan declarados BYREF porque
    // se los pasa al hash-table anti-tamper — o sea el valor real se guarda,
    // pasa por el encrypt/decrypt y se recupera, y el decompile perdio el
    // vinculo entre el que se guarda y el que se lee.
    //
    // El emparejamiento se confirma por el USO: v33 alimenta todas las anims de
    // ATAQUE (FIST +0.6, SWORD +0.25, SKILL_SWORD3 +0.27...) y v38 la de
    // PLAYER_SKILL_ELF1 (+0.25), que es magia.
```

## `src/Combat/Combat_LegacyAttackEffects.cpp`

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

### Línea 47 — antes de `#if !defined(IDA_PORT_00445230)`

```cpp
// IDA: AttackEffect (0x00445230)
// Spawns attack-hit effects and plays sounds based on the attacker's entity type.
// Signature in original binary: __stdcall AttackEffect(CHARACTER *c) — 1 param.
// Declared in functions.h as 3 ints; callers must use that convention.
//
// Main switch on *(short*)(entity+0x02) (Object.Type / MonsterIndex):
//   0x23 '#' DK melee    → CreateEffect(0xBF, pos) + PlayBuffer(0x2E)
//   0x26 '&' DK sword    → CreateEffect(0xC8/0xC9, pos) + PlayBuffer(0x59)
//   0x2A '*' Elf bow     → BMD_TransformPosition(bone 0xB, pos)
//   0x2D '-' Elf arrow   → BMD_TransformPosition(bone 2, pos)
//   0x35 '5' DW          → CreateEffect(0x238, pos)×18 + PlayBuffer
//   0x3D '=' Summoner    → CreateEffect(0xF1, pos) + CreateEffect(0xF0/0x238, pos)
//   0x42 'B'             → CreateEffect(0xF1, pos)
//   0x46 'F' AoE         → CreateEffect(0x4F7, targetPos)×20
//   0x48 'H' Lightning   → CreateJoint(0x4E5, ...)×36
//   0x4D 'M' Lightning   → CreateJoint(0x4E5, ...)×40
//   (most others)        → CreateEffect(0xBF, pos) + PlayBuffer(0x2E)
// Second section: target effects for skill category 0x11 (direct) and 0x03 (magic).
// 2026-08-16: version PARCIAL (374 lineas). Desactivada a favor del port fiel
// de IDA en stubs_IDA_ports.cpp (2043 lineas), que ahora se activa con
// IDA_PORT_00445230 en globals.h.
```

### Línea 447

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// 2026-05-08: Companion-DLL Offsets.h cross-reference — port small functions
// that were truly missing in our build. Sizes per IDA decompile.
// ─────────────────────────────────────────────────────────────────────────────
```

## `src/Combat/Combat_Projectiles.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

## `src/Combat/Combat_Targeting.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

## `src/Combat/Skills.cpp`

### Línea 114 — antes de `#define ENTITY(idx)  ((BYTE*)DAT_07abf5d0 + (idx) * 0x394)`

```cpp
// 2026-05-07: g_EntityBase is never wired to the actual entity array — the real
// base lives in DAT_07abf5d0 (set by WinMain).
```

### Línea 167 en `PacketHandler_0x19` — antes de `*(BYTE*) (caster + 0x301) = (BYTE)(skill_ok != 0);`

```cpp
    // 2026-09-04 FIX: aca habia `(BYTE)(is_pvp == 0)`, o sea el valor INVERTIDO.
    // IDA 0x42BCA0 escribe `sc->SkillSuccess = (TargetKey >> 15) != 0`, y el
    // server MuEmu pone ese bit justamente cuando el skill tuvo exito
    // (`pMsg.target[0] = SET_NUMBERHB(idx) | (type * 0x80)`).
    // Consecuencia: los tres consumidores del flag quedaban al reves --
    // el aura de Greater Defense (MoveCharacter case 27 -> 5x joint 266/sub4)
    // no se creaba nunca, ni el buff de Greater Damage (case 28), ni el
    // congelamiento del Ice Arrow (case 0x33) ni el de Lightning (0x37).
```

### Línea 206 en `PacketHandler_0x19` — antes de `AnimateRemoteSkillCaster97k(caster);`

```cpp
        // UI event 0x3C = ranged hit indicator
        // FUN_00413900(0x3C, caster_idx) — UI dispatch
        //
        // 2026-08-23 CRASH-FIX: aca habia
        //     if (*(BYTE*)(caster + 0x7C) != 0) Entity_ResetToWalk(caster_idx);
        // con DOS errores.  (1) IDA no llama a esa funcion en este camino:
        // `LABEL_107` (0042BCA0 L178-184) solo hace `SetPlayerMagic(sc)` para las
        // entidades que no son el heroe.  La unica que la llama es
        // `SetPlayerBow` en los cases 0x18/0x34/0x33, que ya la invocan bien.
        // (2) le pasaba el INDICE (`caster_idx`) donde la funcion espera el
        // PUNTERO: adentro hace `*(short*)(param_1 + 0x288)`, asi que con
        // idx=124 deferenciaba 124 + 0x288 = 0x304 -> AV.  Verificado contra los
        // registros del crash (`eax=0000007C`, `param1=0x00000304`).
```

### Línea 441

```cpp
// ============================================================
// PacketHandler_0x16  @ 0x0042db60
// Server → Client: kill confirm + EXP gain
// Packet: [C1][len][16][caster_hi][caster_lo][target_hi][target_lo][exp_bytes...][flags]
//
// NOTE: Lines 0–480 are the standard XOR handshake / ACK boilerplate.
//       Real logic begins at decompile offset ~480.
//
// Two modes (byte[3] bit 7):
//   bit7 == 0 → TeleportStart: animate entity moving toward target
//   bit7 == 1 → TeleportEnd:   snap entity to final position
//
// Kill + EXP logic runs for the local player:
//   g_CharData[+0x10] += exp_gained
//   UIChatLogWindow_AddText(exp_gained)  → floating "+EXP" overlay
// ============================================================
// CODIGO MUERTO desde 2026-09-02 — sin callers.
//
// Esta funcion NO era un port de 0x0042DB60.  Interpretaba el 0x16 como
// "teleport begin/end + kill confirm", que no existe en el binario: el raw
// `0042DB60_ReceiveDieExp.c` es la variante chica del 0x9C (Key/Exp/Damage en
// +3..+8, SetPlayerDie o esferas de EXP, y el aviso GlobalText[486]).
//
// Era ademas una landmine: al final hacia `*(BYTE*)(target + 0x2FD) = 1` sobre
// un indice sin validar por abajo, y +0x2FD es el dead_flag — el mismo campo
// que usa como filtro de "vivo" el barrido de sub_45FEC0 (IDA L168 `!v16[18]`),
// o sea marcaba entidades vivas como muertas y las volvia invisibles para el
// reporte de blancos del 0x1D.
//
// El port fiel vive ahora inline en `Net_Process.cpp`, case 0x16.
// (MuEmu no manda este opcode: usa el 0x9C / PMSG_REWARD_EXPERIENCE_SEND.)
```

## `src/Combat/Skills.h`

### Línea 4

```cpp
// PacketHandler_0x16 removida 2026-09-02: era una rama inventada, no un port
// de 0x0042DB60 (ReceiveDieExp).  El port fiel esta inline en Net_Process.cpp,
// case 0x16.  Ver la nota en Skills.cpp.
```

## `src/Combat/Skills_WarriorLegacy.cpp`

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

### Línea 64 — antes de `static void Warrior_SendSkill1E(BYTE skill, BYTE x, BYTE y, BYTE direction,`

```cpp
// IDA: UseSkillWarrior 0x485780, L682-908 -- el sitio que el DLL de inyeccion
// hookea como `SendContinueDeathStab` (Patchs.cpp, 0x00486136).
// `packedOffset` es IDA v316.
//
// 2026-09-03 -- DESVIACION DE PROTOCOLO (servidor MuEmu), la misma que ya
// aplican `Combat_SendDuration1E_97k` y `SendSkillPacket1E_Local`:
// el 0.97k vanilla arma 9 bytes y NO manda la key del objetivo, pero
// PMSG_DURATION_SKILL_ATTACK_RECV (GameServer/SkillManager.h:96) son 11 y el
// server lee `index[]` SIEMPRE:
//     short bIndex = MAKE_NUMBERW(lpMsg->index[0], lpMsg->index[1]);
//     this->UseDurationSkillAttack(..., bIndex, ...);   // SkillManager.cpp:2045
// Con 9 bytes lee esos dos bytes FUERA del paquete: `bIndex` sale basura y
// `MultiSkillAttack -> BasicSkillAttack(aIndex, bIndex, ...)` le pega a otra
// entidad o a ninguna.  Este era el UNICO de los tres emisores de C3:1E que
// habia quedado en la forma vanilla -- descartaba `targetKey` con un
// `(void)targetKey` explicito.
```

### Línea 313 — antes de `// Legacy helper only; no canonical FUN mapping retained here. Previous 'FUN_004742B0'`

```cpp
// ── Entity action stubs (Skills.cpp / Combat.cpp externs) ────────────────────
```

### Línea 452 en `Entity_TeleportAnim` — antes de `char *pcVar1 = (char*)&DAT_07c80110[0];`

```cpp
    // BUG-FIX 2026-05-03: was `while ((int)pcVar1 < 0x7c82cd0)` (absolute source-binary
    // bound). With DAT_07c80110 sized as 1 byte and the loop walking 100 × 0x70 bytes,
    // every teleport effect spawn corrupted the heap. Pool now sized to 100 slots in
    // globals.cpp; bound is iteration count.
```

## `src/Combat/Skills_WizardElf.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Skills_WizardElf.cpp
//
// Extracted from stubs_game.cpp.  Owns wizard/elf skill dispatch helpers.
// Public entry points retain IDA provenance in their leading comments.
```

### Línea 232 — antes de `static BYTE Combat_GetDestValue97kExt(int xPos, int yPos, int xDst, int yDst)`

```cpp
// GetDestValue - DLL Source/Client/Main/Util.cpp:323 (identico al helper del
// cliente 5.2 en source/wsclientinline.h:615): nibble alto = delta X, nibble
// bajo = delta Y, ambos clampeados a [-8, 7].
//
// 2026-09-02: NO se unifico con Combat_GetDestValue97k (Combat.cpp), que si se
// paso a la forma del binario.  Motivo: de este sitio no hay expresion que
// comparar -- el decompile de SkillElf (0x0048BD70 LABEL_68) llega plegado
// (`if...`) y no muestra los appends del payload.  Lo unico seguro es que el
// hook que reemplaza este sitio, CPatchs::SendContinueTripleShot
// (Patchs.cpp:1394-1444), pasa `dest = GetDestValue(x, y, TargetX, TargetY)`.
// Da igual funcionalmente: MuEmu no lee `dis` (SkillManager.cpp:2047 solo
// propaga x, y, dir, angle e index[]).  Se deja como esta hasta poder leer los
// appends en el disassembly.
```

### Línea 311 — antes de `bool __stdcall Combat_UseElfSkillItem(DWORD c, DWORD pItem) {`

```cpp
// IDA: SkillElf @ 0x0048BD70 (6351 bytes) — bool __cdecl SkillElf(DWORD c, DWORD pItem)
// Elf class skill execution. Handles heal, buff, arrow skills.
// Validates target, builds skill packet, handles multi-arrow, spawns VFX.
//
// NOTE: Ghidra shows 63 phantom stack params (in_stack_00000020..in_stack_000000ff)
//   — these are anti-tamper obfuscation, not real parameters.
//   in_stack_00001b48 = c (CHARACTER* pointer, 1st real param)
//   in_stack_00001b4c = pItem (CHARACTER_ATTRIBUTE* pointer, 2nd real param)
//
// The function iterates over skills in the pItem (CharacterAttribute) skill list,
// checking which one matches the currently selected skill on the hero.
// For each matching skill:
//   - If mana is insufficient: tries to use a potion (item slot 3 = arrows/potions)
//   - If AG (SkillMana) is insufficient: returns false
//   - If skill type is 0x18 (arrow/ranged): checks arrow ammo via CheckArrow,
//     validates range, computes facing angle, builds C1 skill packet with
//     XOR encryption, sends it, then calls SetPlayerAttack + CreateArrows
//
// CORRECCION DE TRAZABILIDAD (2026-09-01): este bloque decia
// "IDA: FUN_0048A180 @ 0x0048A180 — SkillElf", y 0x0048A180 es **UseSkillElf**
// (portada como Combat_UseElfSkill en Combat.cpp).  La funcion que se reconstruye
// aca es **SkillElf @ 0x0048BD70**, que es la que Attack llama como
// `SkillElf(c, i + CharacterMachine + 536)` en L1464.  functions.h ya la declaraba
// con la direccion correcta.
// Correspondencia con el decompile de 0x0048BD70:
//   charAttr / pItem        (ITEM* equipado)
//   skillCount         = *(BYTE *)(pItem + 36)     = ITEM::SpecialNum
//   skillId            = *(BYTE *)(pItem + i + 37) = ITEM::Special[i]
//   i                  = i           (L119, la variable de bucle del binario)
//   result             = v110        (el valor de retorno)
//   gridX / gridY      = v44 / v113  (*(_DWORD *)(c + 904) / (c + 908))
//   angle              = v37         (Movement_Tick = CreateAngle @0x0043E050)
//   dir                = v52         (angulo * 0.71111113)
//   skillDistance      = v112        (SkillAttribute[40*skill + 38])
// ~60% of the original decompile is anti-tamper hash table operations
// (HashTable_Insert, FUN_004041e0, HashTable_GetNode, Packet_DecryptByte, Packet_DecryptBuffer,
//  Packet_EncryptBuffer) and XOR key init + dead forward/reverse loops — all skipped.
```

### Línea 376 en `Combat_UseElfSkillItem` — antes de `// The same offsets are used by Attack at 49D278: current mana +0x1e`

```cpp
    // 2026-09-04 FIX (Triple Shot solo se podia lanzar despues de targetear):
    // aca habia un `if (MovementSkillTarget < 0 || >= 400) return false;`.
    // `MovementSkillTarget` (DAT_07D780A0) es el indice de la entidad apuntada:
    // sin blanco vale -1 y el skill no salia; despues de targetear quedaba
    // pegado y por eso funcionaba "un rato" hasta que el indice envejecia.
    // IDA (SkillElf 0x48BD70) no consulta ese global en ningun momento.
```

### Línea 432 en `Combat_UseElfSkillItem` — antes de `BYTE usePkt[6];`

```cpp
            // 2026-07-19: era 6 bytes. PMSG_ITEM_USE_RECV (ItemManager.h) mide
            // 5: header(3) + SourceSlot + TargetSlot. El byte extra dejaba el
            // paquete fuera de spec.
```

### Línea 443 en `Combat_UseElfSkillItem` — antes de `short itemType = *(short*)(OffsetInventoryItems + (size_t)slot * 0x44);`

```cpp
            // Play sound based on item type
            // 2026-08-21: leía DAT_07ea8410, que es un DWORD suelto de 4 bytes —
            // no el pool del inventario (mismo error que ya estaba documentado
            // para FUN_004d23b0).  El grid vive en OffsetInventoryItems, stride 0x44.
```

### Línea 490 en `Combat_UseElfSkillItem` — antes de `float targetWorldX = (float)((int)TargetX) * _DAT_005524f0 + 50.0f;`

```cpp
            // 2026-09-04: se restaura el chequeo de IDA L197-199, que compara
            // contra los globales `TargetX`/`TargetY` -- las coordenadas de
            // GRILLA que deja `CheckTarget` (0x49CAE0).  Ese helper funciona con
            // o sin blanco seleccionado: si `SelectedCharacter == -1` cae al pick
            // de terreno bajo el cursor.  La version anterior tomaba la posicion
            // de la entidad apuntada, que sin blanco no existe.
            //     v35 = c.y - (TargetY * 100.0 + 50.0);
            //     v36 = c.x - (TargetX * 100.0 + 50.0);
            //     if (sqrt(v35*v35 + v36*v36) > Distance * 100.0) -> no dispara
```
