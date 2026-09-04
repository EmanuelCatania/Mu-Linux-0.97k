// Combat.cpp
// Sistema de combate — ataques, animaciones, muerte, skills, ítems en suelo
//
// Cubre los packet handlers de combate (opcodes 0x12..0x20, 0x22, 0x1e)
// y las funciones de lógica de combate del cliente.
//
// ─── ENTITY STRUCT — OFFSETS NUEVOS (combate, stride 0x394) ─────────────────────────────────────────────────────────
//
//   Offset | Type   | Field
//   ──────────────────────────────────────────────────────────────────────────
//   +0x7C  | byte   | attack_state        — 3 = en animación de ataque melee
//   +0x108 | float  | anim_frame_time     — tiempo del frame actual (5.0 = atacando)
//   +0x10C | float  | anim_frame_time_previó? frame previo (guardado al cambiar anim)
//   +0x114 | int*   | bone_transforms_ptr — puntero a array de matrices de huesos
//   +0x164 | float  | attack_intensity    — 1.0 en ataque normal
//   +0x195 | byte   | ragdoll_active      — 1 = física ragdoll activa (post-muerte)
//   +0x198 | float  | ragdoll_vel_x       — velocidad X al morir
//   +0x19C | float  | ragdoll_vel_y       — velocidad Y al morir
//   +0x1A0 | float  | ragdoll_vel_z       — velocidad Z al morir (??rand%10)
//   +0x1A4 | float  | death_pos_x         — posición X al momento de morir
//   +0x1A8 | float  | death_pos_y         — posición Y al momento de morir
//   +0x1AC | float  | death_pos_z         — posición Z al momento de morir
//   +0x21  | byte   | is_attacking        — 1 cuando el servidor confirma ataque
//   +0x274 | byte[] | bone_slot_indices   — índices de huesos de equipo (stride 0x18)
//   +0x2BC | byte   | attack_combo_idx    — = (flags & 0x10 | attack_type) >> 1
//   +0x2EA | byte   | attack_dir          — dirección hacia el objetivo
//   +0x2EC | byte   | attack_cleared      — 0 después de ataque/muerte (reset)
//   +0x2F5 | byte   | is_selected         — 1 = entidad seleccionada como objetivo
//   +0x2FD | byte   | is_dead             — 1 = entidad muerta
//   +0x304 | byte   | physics_active      — ragdoll en zona PvP/mapa específico
//
// ─── ENTITY_SETANIMATION (0x0043e820) ──────────────────────────────────────────────────────────────────────────
//
//   void Entity_SetAnimation(int entity_ptr, uint anim_id):
//     // Tabla de máximos: DAT_05828d58 + 0x26 + entity[+0x02] * 0xBC
//     max_anim = *(short*)(DAT_05828d58 + 0x26 + entity[+0x02] * 0xBC)
//     if (anim_id < max_anim || anim_id == 0x4C || anim_id == 0x4D):
//       if (entity[+0x105] != anim_id):
//         entity[+0x106] = entity[+0x105]   // guardar anim previa
//         entity[+0x10C] = entity[+0x108]   // guardar frame previo
//         entity[+0x105] = anim_id           // nueva anim
//         entity[+0x108] = 0                 // reset frame
//
//   Anims 0x4C y 0x4D son siempre válidas (override global).
//
//   Tabla de anims de combate conocidas:
//     0x57  melee attack (DK/DW/Elf normal)
//     0x5A  entity attack target (opcode 0x1a)
//     0x5C  sit down
//     0x5D  skill anim #1 (DK, no mount)
//     0x5E  skill anim #1 (DW, no mount)
//     0x5F  skill anim #2 (DK)
//     0x60  skill anim #2 (DW)
//     0x61  skill anim #3 (DK)
//     0x62  skill anim #3 (DW)
//     0x65  skill anim #4 (DK)
//     0x66  skill anim #4 (DW)
//     0x67  skill anim #5 (DK)
//     0x68  skill anim #5 (DW)
//     0x69  skill anim #6 (DK)
//     0x6A  skill anim #6 (DW)
//     0x6B  skill anim #7 (DK)
//     0x85  magic staff attack (normal)
//     0x87  magic staff attack (tipo 4)
//     0x89  bow/special attack (DK)
//     0x8A  bow/special attack (DW)
//     0x8B  horse attack #1 (DK)
//     0x8C  horse attack #1 (DW)
//
// ─── ENTITY_MELEEATTACKSTART (legacy helper; no verified FUN mapping) ───────────────────────────────────────────────
//
//   void Entity_MeleeAttackStart(int entity_ptr):
//     FUN_00443e70()                      — reset sonido/efecto global
//     Entity_SetAnimation(entity, 0x57)   — anim de ataque melee
//     entity[+0x108] = 5.0f              — frame time de ataque
//     entity[+0x7C]  = 3                 — attack_state = atacando
//     entity[+0x164] = 1.0f             — attack_intensity
//     Effect_Create(0x498,               // efecto 0x498 = slash/hit particle
//                  world_x, world_z, entity[+0xE8], ...)
//     FUN_00404bc0(0x58, 0, 0)           — UI event: mostrar daño recibido
//
//   Effect_Create @ aprox 0x00460dc0 = Particle_Spawn(effect_id, x, z, height, ...)
//   Efecto 0x498 = impacto de golpe melee (espada/hacha)
//
// ─── ENTITY_ATTACKEFFECT (0x004741e0) ──────────────────────────────────────────────────────────────────────────
//
//   void Entity_AttackEffect(int entity_ptr, int slot):
//     // Obtiene transform del hueso del arma
//     bone_data = DAT_05828d58 + entity[+0x02] * 0xBC   // entity def
//     bone_idx  = entity[+0x274 + slot * 0x18]           // hueso del slot
//     GetBoneTransform(bone_data, bone_idx, entity[+0x114], out_pos, ...)
//
//     // Spawn de 2 partículas de estela de arma
//     color = {1.0, 0.5, 0.2}                            // naranja
//     Particle_Spawn(0x4CF, out_pos, entity[+0x1C], color, slot, 0.0, entity)
//     Particle_Spawn(0x4CF, out_pos, entity[+0x1C], color, slot+2, 0.0, entity)
//
//   Efecto 0x4CF = estela de filo (blade trail), spawn doble para efecto más amplio.
//   entity[+0x1C] = world_z (altura)
//
// ─── OPCODE 0x12 — ENTITY ATTACK EVENT ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x12(void* pkt, uint len)  [FUN_00429690]:
//     Itera count = pkt[1] entidades.
//     Para cada entrada (stride ~0x16 bytes):
//
//     entity_id = byte[-0x12]*256 + byte[-0x11] & 0x7FFF
//     flags     = byte[offset]            // packed byte
//     attack_type = flags >> 4            // 0..15
//     attack_sub  = flags & 0xF           // subtipo
//
//     entity_ptr = FUN_0045bfa0(entity_id, 0x186)
//       — busca entidad tipo 0x186 (player) por ID
//
//     entity[+0x2BC] = (flags & 0x10 | attack_type) >> 1  // combo index
//     entity[+0x1BD] = 0                                    // reset
//     entity[+0x2EA] = byte_dir                             // dirección
//     entity[+0x21]  = 1                                    // is_attacking = true
//     entity[+0x306] = target_grid_x
//     entity[+0x307] = target_grid_y
//     entity[+0x24]  = (angle_byte >> 4 - DAT_0055256c) * DAT_00552844
//
//     // Dispatch por subtipo:
//     switch (attack_sub):
//       1: Entity_MeleeAttackStart(entity)
//              + slot-buffer management (max 9 slots, DAT_07e11db4)
//       2: anim = (attack_type==4) ? 0x87 : 0x85  — Entity_SetAnimation
//       3: anim = (attack_type==4) ? 0x8C : 0x8B  — Entity_SetAnimation
//       4: anim = (attack_type==4) ? 0x8A : 0x89  — Entity_SetAnimation
//
//     // FUN_0043bde0(target_info, entity) = Entity_SetTarget (atualiza objetivo)
//     // Verifica "webzen" token con FUN_004977f0 — anti-cheat check
//     // entity[+0x388/38C] = target_grid X/Y as float
//
//   FUN_0045bfa0 @ 0x0045bfa0 = Entity_FindByIdAndType(id, entity_type)
//   FUN_0043bde0 @ 0x0043bde0 = Entity_SetTarget(target_info, entity)
//   FUN_004977f0 @ 0x004977f0 = AntiCheat_ValidateToken(data, "webzen", '\0')
//
// ─── OPCODE 0x13 — ENTITY POSITION (COMBAT MOVE) ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x13(void* pkt, ...)  [FUN_0042a230]:
//     Itera count = pkt[4] entidades (stride 9 bytes):
//
//     entity_id = byte[-1]*256 + byte[-2] & 0x7FFF
//     entity_ptr = FUN_0045ccf0(move_type, grid_x, grid_y, entity_id, 0)
//       — busca/spawna entidad por ID, tipo y posición
//
//     FUN_0043bde0(flags, entity)    — Entity_SetTarget (actualiza objetivo del mapa)
//     entity[+0x388] = (float)grid_x  // cached_wp_x
//     entity[+0x38C] = (float)grid_y  // cached_wp_y
//     entity[+0x306] = grid_x (byte)
//     entity[+0x307] = grid_y (byte)
//     entity[+0x24]  = facing_angle
//
//     Si entity_id >> 15 == 0 (not NPC-high flag):
//       FUN_0043d3e0 + HashTable ops — actualiza posición "segura"
//
//   FUN_0045ccf0 @ 0x0045ccf0 = Entity_FindOrSpawn(move_type, gx, gy, id, flag)
//
// ─── OPCODE 0x15 — ENTITY SPAWN / STATE UPDATE ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x15(void* pkt)  [FUN_0042acc0]:
//     entity_id   = byte[3]*256 + byte[4] & 0x7FFF
//     flags_byte  = byte[5]
//     entity_ptr  = Entity_FindById(entity_id)
//
//     bits de flags_byte:
//       bit 4 = flag_4 (local_28)
//       bit 5 = flag_5 (local_1c)
//       bit 6 = flag_6 (local_24)
//       bit 7 = flag_7 (local_20, warp)
//
//     Si bit15 de entity_id set (warp/teleport):
//       FUN_00444b60(entity, target_pos)   — Entity_TeleportStart
//       FUN_004792c0(entity+4, target, ...) — TeleportAnimation
//       entity[+0x2BE] = (ushort)new_pos
//       Si entity_id != DAT_05826cac (local player): return
//       // Si es el jugador local: actualiza g_CharData también
//     Else:
//       Actualiza posición normal (entity[+0x10/14/24])
//       FUN_00444b60 + state update
//
//   DAT_05826cac = local player entity ID
//   FUN_00444b60 @ 0x00444b60 = Entity_TeleportStart
//
// ─── OPCODE 0x16 — SKILL USE ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x16(void* pkt, int param_2)  [FUN_0042db60]:
//     Si param_2 == 0 (primera llamada):
//       Construye respuesta handshake con clave XOR {0xe7,0x6d,0x3a,...}
//       (misma clave que login, 28 bytes)
//       opcode de respuesta: [0xC1][4][0xF1][1] XOR-encriptado — enviar
//       — Este es el ACK de "request skill use" al servidor
//
//     Si param_2 != 0 (procesado):
//       Proceso normal del skill:
//       - Lee skill_id, target_id, caster_id del paquete
//       - Effect_Create(effect_id, ...) — spawn efecto visual del skill
//       - FUN_0043e820(entity, skill_anim) — animación del caster
//       - Actualiza buffers de skill en entidades afectadas
//
//   La función tiene 582 líneas y maneja todos los skills del juego.
//   Pattern: param_2==0 — ACK path; param_2!=0 — effect application path
//   (mismo patrón que PacketHandler_0x1e y PacketHandler_0x19)
//
// ─── OPCODE 0x17 — ENTITY DEATH ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x17(void* pkt)  [FUN_0042f030]:
//     entity_id  = byte[3]*256 + byte[4]
//     entity_ptr = DAT_07abf5d0 + Entity_FindById(entity_id) * 0x394
//
//     entity[+0x2FD] = 1    // is_dead = true
//     entity[+0x2EC] = 0    // attack_cleared = 0
//
//     // F??sica ragdoll solo en sub-estado 0x0B..0x10:
//     if (g_GameSubState >= 0x0B && g_GameSubState <= 0x10):
//       tile_flags = DAT_0838bc70[FUN_004f6c40(grid_x, grid_y)]
//       if (tile_flags & 0x20):    // tile flag = "ragdoll zone"
//         entity[+0x304] = 0
//         entity[+0x195] = 1       // ragdoll_active = true
//         entity[+0xD8]  = rand()%10 + offset_x     // x_offset random
//         entity[+0xCC]  = rand()%20 + offset_y     // y_offset random
//         entity[+0x1A0] = ±(rand()%10 + 2.0f)     // vel_z (knockback)
//           negativo si tile bit izquierdo set, positivo si derecho
//         entity[+0x198] = 0        // vel_x = 0
//         entity[+0x19C] = 0        // vel_y = 0
//         entity[+0x24]  = entity[+0x1A0]  // facing = vel_z (para rotación)
//         entity[+0xC0]  = rand()%6 + 1.5f  // spin_rate
//         entity[+0xC4]  = ±(rand()&1) + 1.3f // spin_y
//         entity[+0x1A4] = entity[+0x10]   // death_pos_x = world_x
//         entity[+0x1A8] = entity[+0x14]   // death_pos_y = world_y (= 45 decimal)
//         entity[+0x1AC] = entity[+0x18]   // death_pos_z = world_z
//         entity[+0x1A8] = rand()%45       // y override (altura de lanzamiento)
//
//     Si entity_ptr == DAT_07abf5d8 (jugador local):
//       FUN_0047eb80()    // PlayerDeath_Handler
//
//   FUN_004f6c40 @ 0x004f6c40 = Terrain_GetTileIndex(grid_x, grid_y)
//   DAT_0838bc70 = terrain tile flags array (per-tile bytes)
//   FUN_0047eb80 @ 0x0047eb80 = PlayerDeath_Handler
//
// ─── OPCODE 0x18 — ENTITY SKILL ANIMATION ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x18(void* pkt)  [FUN_0042b4f0]:
//     entity_id  = byte[3]*256 + byte[4]
//     skill_id   = byte[6]
//     entity_ptr = DAT_07abf5d0 + Entity_FindById(entity_id) * 0x394
//
//     // Actualiza posición desde grid guardado en entity:
//     entity[+0x24]  = (byte[5] - DAT_0055256c) * DAT_00552844  // facing
//     entity[+0x10]  = entity[+0x306] * tile_size + offset       // world_x
//     entity[+0x14]  = entity[+0x307] * tile_size + offset       // world_y
//     entity[+0x2EC] = 0                                          // attack_cleared
//
//     switch (skill_id):
//       0x12:  Entity_SetAnimation(entity, 0x5C)  // sit anim
//              UI_FireEvent(0x51)                 // sit UI update
//
//       0x64:  FUN_00444410(entity)               // Entity_SelectTarget
//              DAT_05826D28 = entity_idx           // global selected entity
//              entity[+0x2F5] = 1                  // is_selected = true
//              entity[+0x108] = 0                  // reset frame
//              entity[+0x310] = 0xFFFF             // clear attack target
//
//       0x65:  mismo que 0x64 (death select?)
//
//       0x66:  Entity_SetWalkAnimation(entity)    // reset to walk
//       0x67:  Entity_SetWalkAnimation(entity)
//
//       0x6C:  (break, sin acción)
//
//       0x6D:  // DK: 0x8B, DW: 0x8C (horse attack #1)
//              if (entity[+0x1BC] & 7) != 2: anim = 0x8B
//              else:                          anim = 0x8C
//              Entity_SetAnimation(entity, anim)
//
//       0x6E:  anim = (entity[+0x1BC] & 7 != 2) ? 0x89 : 0x8A
//       0x6F:  anim = (entity[+0x1BC] & 7 != 2) ? 0x5D : 0x5E
//       0x70:  anim = (entity[+0x1BC] & 7 != 2) ? 0x5F : 0x60
//       0x71:  anim = (entity[+0x1BC] & 7 != 2) ? 0x61 : 0x62
//       0x72:  anim = (entity[+0x1BC] & 7 != 2) ? 0x67 : 0x68
//       0x73:  anim = (entity[+0x1BC] & 7 != 2) ? 0x65 : 0x66
//       0x74:  anim = (entity[+0x1BC] & 7 != 2) ? 0x69 : 0x6A
//       0x75:  anim = (entity[+0x1BC] & 7 != 2) ? 0x6B : ...
//
//       default: Entity_SetAnimation(entity, skill_id)  // anim directa
//
//   Entity[+0x1BC] & 7 = clase de la entidad (0=DK, 1=DW, 2=Elf, 3=MG)
//   FUN_00444410 @ 0x00444410 = Entity_SelectTarget(entity)
//   DAT_05826D28 = global índice de entidad atacante/seleccionada
//
// ─── OPCODE 0x1A — ATTACK TARGET SET ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x1a(void* pkt)  [FUN_0042d780]:
//     entity_id      = byte[3]*256 + byte[4]
//     entity_idx     = Entity_FindById(entity_id)
//     entity_ptr     = DAT_07abf5d0 + entity_idx * 0x394
//
//     DAT_05826D28 = entity_idx   // global: índice del atacante activo
//
//     FUN_004741e0(entity_ptr, 0)  // Entity_AttackEffect (weapon bone trail x2)
//     UI_FireEvent(0x58)           // mostrar UI de combate (damage overlay)
//     Entity_SetAnimation(entity_ptr, 0x5A)  // anim 0x5A = target acquired
//
//     entity[+0x302] = nuevo estado (HashTable update anti-tamper)
//
// ─── OPCODE 0x1E — SKILL REQUEST (client→server ACK path) ───────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x1e(void* pkt, ?, int first_call)  [FUN_0042cd10]:
//     Mismo patrón que opcode 0x16 y 0x19:
//     Si first_call == 0: construye y envía ACK con XOR key {0xe7,0x6d,...}
//     Si first_call != 0: procesa datos del skill (580 líneas)
//
// ─── OPCODE 0x1F — SKILL RESULT ──────────────────────────────────────────────────────────────────────────
//
//   FUN_0042a530 — no decompilado aún, ~similar a 0x20
//
// ─── OPCODE 0x20 — GROUND ITEM SPAWN ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x20(void* pkt)  [FUN_0042f240]:
//     count = byte[4]
//     Itera count entradas:
//
//     entity_id = byte[0]*256 + byte[1] & 0x7FFF
//     grid_x    = byte[2]
//     grid_y    = byte[3]
//     is_mine   = entity_id >> 15    // bit15 = ítem del jugador local
//     item_raw  = &byte[9-stride_offset]
//
//     world_x = (grid_x + DAT_00552504) * DAT_005524f0
//     world_y = (grid_y + DAT_00552504) * DAT_005524f0
//
//     FUN_005032f0(&DAT_07e127f8 + entity_id*0x204, item_raw, world_x, is_mine)
//       — GroundItem_Spawn(slot_ptr, item_data, pos, ownership_flag)
//
//     item_type = Item_GetType(item_raw)
//     stride    = (item_type == 0x1CF) ? 9 : 8  // Jewel of Chaos = stride 9
//
//   DAT_07e127f8 = ground items array (stride 0x204 por slot, max 1000 slots)
//   FUN_005032f0 @ 0x005032f0 = GroundItem_Spawn(slot, item_raw, world_pos, is_mine)
//
// ─── OPCODE 0x22 — PLAYER HP/MP UPDATE ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x22(uint param_1)  [FUN_0042f360]:
//     byte[3]:
//       0xFF: no-op
//       0xFE: HashTable decode de g_CharData — actualiza stats del personaje
//       Otro: actualiza HP/MP en tabla de inventario/CharData por slot < 0x0C
//
//   Probablemente: opcode de respuesta a poción usada o daño recibido.
//
// ─── OPCODE 0x23 — EQUIP RESPONSE / ITEM RESULT ──────────────────────────────────────────────────────────────────────────
//
//   void PacketHandler_0x23(void* pkt)  [FUN_0042f690]:
//     byte[3] == 0: FUN_004cd3b0() + _DAT_00559680 = -1  — inventory reset
//     byte[4] < 0x0C: decode g_CharData — equipment slot update
//
//   FUN_004cd3b0 @ 0x004cd3b0 = Inventory_Reset (limpia estado del inventario UI)
//
// ─── PACKET DE ATAQUE CLIENTE→SERVIDOR ──────────────────────────────────────────────────────────────────────────
//
//   El cliente envía ataques (no hay función explícita documentada aún, pero
//   se infiere de los handlers de respuesta):
//
//   [0xC1][len][0x1C][target_id_high][target_id_low][dir][skill_id]
//   o similar — servidor responde con opcode 0x12 (attack event) para todos
//   los clientes en rango.
//
// ─── PIPELINE DE COMBATE COMPLETO ──────────────────────────────────────────────────────────────────────────
//
//   1. Jugador hace click en monstruo — Input.cpp captura click
//   2. Cliente envía paquete de ataque (0x1C o F1/xx)
//   3. Servidor procesa daño con algoritmo de stats (CharData_RecalcStats)
//   4. Servidor envía a todos en rango:
//      — 0x13 (entity position update, monstruo se mueve hacia atacante)
//      — 0x12 (entity attack event, activa anim de ataque)
//         ─── PacketHandler_0x12 — Entity_MeleeAttackStart (si sub==1)
//              ─── Entity_SetAnimation(0x57)
//              ─── Particle_Spawn(0x498) — efecto de golpe
//              ─── UI_FireEvent(0x58)    — UI de daño
//      — 0x1A (attack target set, muestra quién ataca a quién)
//      — 0x17 (entity death, si HP == 0)
//         └── entity[+0x2FD] = 1
//         ─── Ragdoll si zona PvP (tile flag 0x20)
//         ─── PlayerDeath si es local player
//   5. Skills/magia:
//      — Cliente envía skill request (opcode 0x1E)
//         ─── PacketHandler_0x1e manda ACK con XOR handshake
//      — Servidor responde con 0x16/0x18 (skill use + anim)
//         ─── PacketHandler_0x18 — Entity_SetAnimation(skill_anim)
//   6. ítems dropean:
//      — Servidor envía 0x20 (ground item spawn)
//         ─── GroundItem_Spawn(DAT_07e127f8 + id*0x204, ...)
//
// ─── GLOBALS ──────────────────────────────────────────────────────────────────────────
//
//   DAT_05826D28   — índice de entidad en combate activa (atacante / seleccionada)
//   DAT_05826CAC   — entity ID del jugador local
//   DAT_07E127F8   — ground items array (stride 0x204, max 1000)
//   DAT_05828D58   — entity definitions table (stride 0xBC, +0x26 = max_anim)
//   DAT_0838BC70   — terrain tile flags array (per-tile bytes, flag 0x20 = ragdoll)
//   DAT_0055256C   — angle bias constant (offset para normalizar ??ngulo byte)
//   DAT_00552844   — angle scale (byte — radianes: (byte - bias) * scale)
//   DAT_005524F0   — tile_size (grid — world units)
//   DAT_00552504   — tile_offset (0.5 en grid)
//
// ─── CROSS-REFERENCE ──────────────────────────────────────────────────────────────────────────
//
//   PacketHandler_0x12  @ 0x00429690  — Entity attack event
//   PacketHandler_0x13  @ 0x0042a230  — Entity combat position
//   PacketHandler_0x15  @ 0x0042acc0  — Entity spawn/teleport
//   PacketHandler_0x16  @ 0x0042db60  — Skill use (ACK + apply)
//   PacketHandler_0x17  @ 0x0042f030  — Entity death + ragdoll
//   PacketHandler_0x18  @ 0x0042b4f0  — Skill animation dispatch
//   PacketHandler_0x19  @ 0x0042bca0  — Magic/skill effect (889 lines)
//   PacketHandler_0x1a  @ 0x0042d780  — Attack target set
//   PacketHandler_0x1e  @ 0x0042cd10  — Skill request (ACK path)
//   PacketHandler_0x1f  @ 0x0042a530  — Skill result (no decompilado)
//   PacketHandler_0x20  @ 0x0042f240  — Ground item spawn
//   PacketHandler_0x22  @ 0x0042f360  — HP/MP update
//   PacketHandler_0x23  @ 0x0042f690  — Equip/inventory result
//   Entity_SetAnimation @ 0x0043e820  — set anim con bounds check
//   Entity_MeleeAttackStart — legacy helper; no verified FUN mapping.
//   Entity_AttackEffect @ 0x004741e0  — weapon bone trail particles
//   Entity_FindByIdAndType @ 0x0045bfa0
//   Entity_FindOrSpawn  @ 0x0045ccf0
//   Entity_SetTarget    @ 0x0043bde0
//   Entity_TeleportStart @ 0x00444b60
//   Entity_SelectTarget @ 0x00444410
//   Entity_SetWalkAnimation @ 0x004430c0
//   PlayerDeath_Handler @ 0x0047eb80
//   Terrain_GetTileIndex @ 0x004f6c40
//   GroundItem_Spawn    @ 0x005032f0
//   Particle_Spawn      @ 0x00460dc0
//   AntiCheat_Validate  @ 0x004977f0
//   Inventory_Reset     @ 0x004cd3b0
//   CharData_RecalcStats @ 0x0047e3c0  (ver UI.cpp)

#include "stdafx.h"
#include "Item/Item_Equip.h"
#include "Combat/Combat.h"
#include "Net/Net.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);

static BYTE Combat_ResolveQueuedSkillId97k();
static int Combat_GetSkillRange97k(int skillType);
static void Combat_SeedRuntimeState97k(int skillType, int targetIdx);
static void Combat_SendPartyRecall97k(char* entity, int targetIdx,
                                      BYTE destinationX, BYTE destinationY);
// Emisor puro del C1:09:1E nativo del 0.97k.  Los tres bytes finales son los
// campos 6/7/8 del payload; las etiquetas Angle/Dest/Tpos proceden de 5.2 y no
// se usan como semantica del port.  La animacion la hace cada case por su cuenta
// porque IDA usa combinaciones distintas
// (SetAction solo, SetAttackSpeed+SetAction, o SetPlayerMagic).
static void Combat_SendDuration1E_97k(char* entity, int skillType,
                                      BYTE field6, BYTE field7, BYTE field8,
                                      int packetX = -1, int packetY = -1);
static void Combat_SendPlainPacket97k(BYTE* pkt, int len);

// Base de la grilla de inventario — la usa el feedback sonoro del mana-scroll de Attack.
// Defined in src/Render/HUD_Pass3.cpp.
extern "C" BYTE OffsetInventoryItems[];

// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 6475-7690 (1216 lines)
// Combat_SendMovePathPacket (Send_MovePacket), Combat_DispatchHeroSkillAttack (Attack), Combat_CheckArrowRequirement (CheckArrow),
// Combat_UseElfSkill (UseSkillElf stub), Combat_ProcessQueuedAction (Action big switch),
// + Send_MovePacket_Player_legacy_stub, FUN_004f6c30 (Terrain_GetAttrDirect)
// =============================================================================
// IDA: FUN_00491c40 @ 0x00491C40 — Send_MovePacket(entity_ptr, player_entity_ptr)
// Sends opcode 0x10 movement packet: C1 len 10 wp_count target_x target_y facing path[wp_count]
// Codifica con XOR usando la clave hardcodeada de 32 bytes. Saltea si la entidad tiene el bit 0x20 en +0x78.
// wp_count se limita a 0xe. Setea DAT_00559bec = pkt_size_code.
void __cdecl Combat_SendMovePathPacket(int param_1, int param_2)
{
    // Saltea si la entidad en param_2+0x78 tiene el flag 0x20 seteado (entidad ocupada/bloqueada)
    if ((*(unsigned char*)(param_2 + 0x78) & 0x20) == 0x20)
        return;

    unsigned char wpCount = *(unsigned char*)(param_1 + 0x356);
    if (wpCount < 3)
        DAT_00559bec = 0;
    else if (wpCount == 3)
        DAT_00559bec = 5;
    else
        DAT_00559bec = (unsigned int)wpCount * 3 + 4;

    if (wpCount == 0)
        return;
    if (wpCount > 0xe)
        wpCount = 0xe;

    // 2026-05-05 BUG-FIX: el packet de move tenía la nibble inversa y length
    // fijo. Per IDA decompile Combat_SendMovePathPacket + server CGMoveRecv:
    //   path[0] = (dir0 << 4) | (wpCount - 1)
    //   path[1..] cada byte packs 2 dirs: high=dir[2k+1], low=dir[2k+2]
    //   total length = 5 + ((wpCount >> 1) + 1) bytes
    // Antes mandábamos: path[0] = (wpCount << 4) | dir0  — server leía Dir=wpCount
    // y PathCount=dir0 — no path procesado — server ignoraba y char snapeaba.
    unsigned char pkt[16];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0xC1;
    // length set abajo
    pkt[2] = 0x10;        // PROTOCOL_CODE1 (move opcode)
    // 2026-05-05 BUG-FIX: IDA decompile Combat_SendMovePathPacket muestra que pkt[3]/pkt[4]
    // son `entity[+0x357]` y `entity[+0x366]` = path_wp_x[0]/path_wp_y[0]
    // (= START de la path = current grid pos), NO entity[+0x306]/[+0x307]
    // (= target del último move server-confirmed). Server lee pkt[3]/[4] como
    // PathX[0] y walks PathX[1..PathCount-1] aplicando los dirs. Si pkt[3]/[4]
    // era el target, server simulaba walk DESDE target — char acababa en
    // posición incorrecta — server snap-back con GCTeleportSend.
    pkt[3] = *(unsigned char*)(param_1 + 0x357); // path_wp_x[0] = start grid X
    pkt[4] = *(unsigned char*)(param_1 + 0x366); // path_wp_y[0] = start grid Y

    // Compute directions (8-dir Mu standard: 0=W, 1=SW, 2=S, 3=SE, 4=E, 5=NE, 6=N, 7=NW).
    BYTE pathDir[16] = {};
    int  numDir = 0;
    for (int i = 0; i < wpCount && i < 16; i++) {
        BYTE px = (i == 0)
            ? *(unsigned char*)(param_1 + 0x306)
            : *(unsigned char*)(param_1 + 0x357 + i - 1);
        BYTE py = (i == 0)
            ? *(unsigned char*)(param_1 + 0x307)
            : *(unsigned char*)(param_1 + 0x366 + i - 1);
        BYTE nx = *(unsigned char*)(param_1 + 0x357 + i);
        BYTE ny = *(unsigned char*)(param_1 + 0x366 + i);
        int dx = (int)nx - (int)px;
        int dy = (int)ny - (int)py;
        // 2026-05-05 BUG-FIX: dir encoding debe matchear server's RoadPathTable
        // (Util.cpp:19): { (-1,-1), (0,-1), (1,-1), (1,0), (1,1), (0,1), (-1,1), (-1,0) }.
        // Antes el mapping estaba rotado +1 — server walk a dirección equivocada
        // — tiles bloqueadas — server respondía con 0x11 snap-back.
        BYTE dir = 0;
        if      (dx < 0 && dy < 0)  dir = 0;  // NW
        else if (dx == 0 && dy < 0) dir = 1;  // N
        else if (dx > 0 && dy < 0)  dir = 2;  // NE
        else if (dx > 0 && dy == 0) dir = 3;  // E
        else if (dx > 0 && dy > 0)  dir = 4;  // SE
        else if (dx == 0 && dy > 0) dir = 5;  // S
        else if (dx < 0 && dy > 0)  dir = 6;  // SW
        else if (dx < 0 && dy == 0) dir = 7;  // W
        pathDir[numDir++] = dir;
    }

    // path[0] = (dir0 << 4) | (wpCount - 1) — HIGH nibble = direction,
    // LOW nibble = wpCount-1 (server: lpObj->Dir=path[0]>>4, PathCount=path[0]&0xF).
    pkt[5] = (BYTE)((pathDir[0] & 0xF) << 4) | (BYTE)((wpCount - 1) & 0xF);

    // path[1..] each byte packs 2 dirs (HIGH=dir[2k+1], LOW=dir[2k+2]).
    // quedan numDir-1 direcciones por empaquetar, arrancando en pathDir[1].
    for (int i = 1; i < numDir && i < 15; i++) {
        int byteIdx = 5 + ((i + 1) >> 1);
        if ((i & 1) == 1) {
            pkt[byteIdx] = (BYTE)((pathDir[i] & 0xF) << 4);  // high nibble
        } else {
            pkt[byteIdx] |= (BYTE)(pathDir[i] & 0xF);        // low nibble
        }
    }

    // Total length = 5 (C1 LL 10 X Y) + ((wpCount >> 1) + 1) bytes for path.
    unsigned int payloadLen = 5 + ((wpCount >> 1) + 1);
    pkt[1] = (BYTE)payloadLen;

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
    static const BYTE s_MoveKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };
    for (unsigned int i = 3; i < payloadLen; i++) {
        pkt[i] ^= pkt[i - 1] ^ s_MoveKey[i & 0x1f];
    }

    Net_SendBuf((const char*)pkt, (int)payloadLen);

    // IDA 00491C40, justo después del camino de envío por la red:
    // acá se activa la ruta generada localmente. Player_InputTick sólo la
    // avanza/interpola mientras este byte esté seteado.
    if (DAT_083a7c24 != 113)
        *(unsigned char*)(param_1 + 748) = 1;
}

// ──────────────────────────────────────────────────────────────────────────
// IDA: FUN_0049cbf0 @ 0x0049CBF0 — Attack(c)  [PORTED 2026-05-05]
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
// CALLER: MoveCharacter (FUN_00449900) — llama esta función cuando el hero
// tiene flag de attack activo. También llamada desde UseSkillWarrior y
// UseSkillElf como fallback continuation.

// Helpers: real names exposed via functions.h
//   Path_FindRoute = PathFinding2(sx, sy, tx, ty, path_buf, radius) — uint
//   FUN_00483160 = CheckAttack() — uint (0 if locked)
//   FUN_0049cae0 = CheckTarget_stub(c) — bool
//
// HeroKey se lee del campo +0x1DC de la entidad del héroe (= g_HeroKey que asigna
// Net_Process at JoinServer ACK; see Net_Process.cpp:1294).

// 2026-09-01: aca vivian dos statics locales (`g_dwLatestMagicTick_Attack` y
// `g_dwLatestTeleportRequest_Attack`) que reemplazaban a los globals del binario
// porque se creia que sus direcciones aliaseaban timers de UI/NPC.  Es falso:
//   g_dwLatestMagicTick  = DAT_05826cf4 (ya usado por UseSkillWarrior/Wizard)
//   dword_7E11DC8 / DC4  = DAT_07e11dc8 / DAT_07e11dc4 — los escribe
//                          ReceiveTeleport (0x428210, xref 0x428EBF) y los leen
//                          Attack (0x4AB5E7) y CheckGate (0x4AC6DE).
// Con los statics el cooldown de teleport de Attack no compartia estado con el
// que arma el servidor, asi que los dos gates corrian por separado.

// 0049CCAA..0049CCF1.  The original keeps these independently of the UI
// timers: they measure how long the right button has remained down.  Keeping
// them local is appropriate for the reconstructed client, where the original
// addresses alias unrelated UI state.
static float g_RightButtonPressSeconds_Attack = 0.0f;
static float g_RightButtonReleaseTime_Attack = 0.0f;

extern "C" int g_MouseOnWindow;   // Game/Player_InputTick.cpp

// Correspondencia con el decompile de IDA (0049CBF0), para que los nombres
// semanticos de abajo sigan siendo rastreables:
//   entity        = c            (CHARACTER*, el heroe)
//   iType         = iType / v13  (id del skill del slot equipado)
//   bSuccess      = Success      (latch del boton derecho)
//   piManaLocal   = piMana       ·  piSkillManaLocal = piSkillMana
//   scrollSlot    = LODWORD(y1)  (retorno de sub_482BE0 / GetItemSlot)
//   movementState = v16          ·  actionState = v17
// Globals del binario original (direccion verificada con ida_xrefs_to):
//   EditFlag 0x07E11D30 · MouseOnWindow 0x07D78094 · Attacking 0x00559C58
//   RButtonPressTime 0x07E11D58 · RButtonPopTime 0x07E11D54
//   SelectedCharacter 0x00559C50 · m_bAutoAttack 0x07E11E18
//   WorldTime 0x05826E08 · MovementSkillTarget 0x07D780A0
//   dword_7D7809C 0x07D7809C · CurrentSkill 0x05826D10
//   g_dwLatestMagicTick 0x05826CF4
// ══════════════════════════════════════════════════════════════════════════════
// Bloques del CFG de Attack (0x0049CBF0) reconstruidos como funciones propias.
// El binario los encadena con `goto`; separarlos evita saltos sobre
// inicializaciones sin cambiar el orden ni las condiciones.
//   AF_RETURN     = el bloque termino el tick (los `return` / LABEL_146 de IDA)
//   AF_LABEL_322  = el bloque hizo `goto LABEL_322` (IDA L1881 / L1886)
// ══════════════════════════════════════════════════════════════════════════════
enum Attack_Flow97k { AF_RETURN = 0, AF_LABEL_322 = 1 };

static bool Attack_OutOfRange97k(const char* entity, int range);
static void Attack_UseManaScroll97k();
static Attack_Flow97k Attack_Label240_97k(char* entity, int iType);
static Attack_Flow97k Attack_ElfBranch97k(char* entity, int iType);
static void Attack_WarriorBranch97k(char* entity);
static void Attack_WizardBranch97k(char* entity);
static void Attack_SoulBarrier97k(char* entity, int iType, bool hasTarget);
static void Attack_Label1158_97k(char* entity, int iType, bool hasTarget);
static void Attack_Label1585_97k(char* entity, int iType, bool hasTarget);

// Chequeo de alcance que IDA repite en LABEL_240 (L1811-1813), case 42 (L3766),
// case 55 (L4916), case 16 (L7171) y LABEL_1158 (L7817):
//   sqrt((c+20 - (TargetY*100+50))^2 + (c+16 - (TargetX*100+50))^2) > range*100
static bool Attack_OutOfRange97k(const char* entity, int range)
{
    const float tx = (float)(int)DAT_07e016c0 * 100.0f + 50.0f;   // IDA: TargetX
    const float ty = (float)(int)DAT_07e016c4 * 100.0f + 50.0f;   // IDA: TargetY
    const float dx = *(float*)(entity + 16) - tx;
    const float dy = *(float*)(entity + 20) - ty;
    const float maxDistance = (float)range * 100.0f;
    return (dx * dx + dy * dy) > (maxDistance * maxDistance);
}

// IDA L1507-1749 (y los clones identicos de los cases 41/42/48/55/56 y del
// bloque DW/MG en L5954-6196): sin mana suficiente, intenta gastar un scroll del
// inventario y SIEMPRE aborta el skill.
static void Attack_UseManaScroll97k()
{
    const int scrollSlot = (int)FUN_00482be0(3);   // IDA: LODWORD(y1) = sub_482BE0(3)
    if (scrollSlot == -1)
        return;
    if (WarehouseOpened || TradeOpened) {
        UIChatLogWindow_AddText("ERROR", GlobalText[474], 2);
        return;
    }
    if (EnableUse > 0)
        return;
    EnableUse = 10;
    // PMSG_ITEM_USE_RECV: [C1][05][26][SourceSlot][TargetSlot]
    unsigned char pkt[5];
    pkt[0] = 0xC1; pkt[1] = 5; pkt[2] = 0x26;
    pkt[3] = (unsigned char)(scrollSlot + 12);
    pkt[4] = 0;
    Net_SendSmallPacket(pkt, 5);
    const int itemType = *(int*)(OffsetInventoryItems + (size_t)scrollSlot * 0x44);
    if (itemType == 448)                       FUN_00404bc0(33, 0, 0);
    else if (itemType >= 449 && itemType <= 457) FUN_00404bc0(32, 0, 0);
}

// Siembra que IDA repite en LABEL_240 (L1784-1789), case 16 (L7136-7141) y
// LABEL_1158 (L7781-7787).  Ojo el orden: primero limpia, despues escribe.
static void Attack_SeedSkillTarget97k()
{
    if (SelectedCharacter == -1)
        return;
    DAT_07d78098 = 1;                                        // IDA: dword_7D78098
    DAT_07d7809c = 0;                                        // IDA L1785
    DAT_07d780a0 = 0;                                        // IDA L1786: MovementSkillTarget
    const BYTE slot = Hero ? *(BYTE*)(Hero + 913) : 0;       // IDA: v133 / v545 / v629
    DAT_07d780a0 = (DWORD)SelectedCharacter;
    DAT_07d7809c = slot;
}

// Envio comun de C1:06:19 con el gate de 300 ms sobre g_dwLatestMagicTick.
static void Attack_SendSkill19_97k(int iType, WORD key)
{
    const DWORD now = GetTickCount();
    DWORD elapsed = now - DAT_05826cf4;                      // IDA: g_dwLatestMagicTick
    if (elapsed > 0x80000000u) elapsed = (DWORD)(-(int)elapsed);   // IDA: abs32
    if ((int)elapsed <= 300)
        return;
    DAT_05826cf4 = GetTickCount();
    BYTE packet[6] = { 0xC1, 6, 0x19, (BYTE)iType,
                       (BYTE)(key >> 8), (BYTE)key };
    Net_SendSmallPacket(packet, sizeof(packet));
}

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
static BYTE Combat_GetDestValue97k(int xPos, int /*yPos*/, int xDst, int /*yDst*/)
{
    const BYTE delta = (BYTE)(xDst - xPos);
    return (BYTE)(((BYTE)(delta + 8) << 4) | ((BYTE)(delta - 8) & 0x0F));
}

static BYTE Attack_PackDestination97k(const char* entity)
{
    return Combat_GetDestValue97k((int)*(DWORD*)(entity + 904),
                                  (int)*(DWORD*)(entity + 908),
                                  (int)DAT_07e016c0,
                                  (int)DAT_07e016c4);
}

// Byte `index` del C3:1E — DLL, los 13 hooks `SendContinue*` de Patchs.cpp:
//     index = CheckAttack() ? *(short *)(CharactersClient + 916*Sel + 0x1DC) : -1;
static int Combat_SkillTargetIndex97k(void)
{
    if (SelectedCharacter < 0 || !CharactersClient)
        return -1;
    if (!FUN_00483160())                                  // CheckAttack()
        return -1;
    return (int)*(short*)((char*)(uintptr_t)CharactersClient
                          + 916 * (int)SelectedCharacter + 0x1DC);
}

// ── IDA L1781-2158: LABEL_240 (rama Elf de skills bajos / >36 no-52) ──────────
static Attack_Flow97k Attack_Label240_97k(char* entity, int iType)
{
    Attack_SeedSkillTarget97k();                              // IDA L1782-1790
    const int range = Combat_GetSkillRange97k(iType);         // IDA L1810
    if (Attack_OutOfRange97k(entity, range)) {                // IDA L1813
        if (SelectedCharacter != -1 &&
            *((BYTE*)(uintptr_t)CharactersClient + 916 * (int)SelectedCharacter + 132) == 1) {
            if (Path_FindRoute(*(int*)(entity + 904), *(int*)(entity + 908),
                               (int)DAT_07e016c0, (int)DAT_07e016c4,
                               (unsigned char*)(entity + 852), (float)range)) {
                entity[748] = 1;                              // IDA L1827
                entity[749] = 5;                              // IDA L1828
            }
        }
        return AF_RETURN;                                     // IDA L1834/1844
    }
    // IDA L1862: CheckWall(c+904, c+908, TargetX, TargetY) && SelectedCharacter != -1
    if (Path_IsLineClear(*(int*)(entity + 904), *(int*)(entity + 908),
                         (int)DAT_07e016c0, (int)DAT_07e016c4)
        && SelectedCharacter != -1) {
        BYTE* const target = (BYTE*)(uintptr_t)CharactersClient +
                             916 * (int)SelectedCharacter;    // IDA: CharactersClient[SelectedCharacter]
        if (target[132] == 1 && iType >= 26 && iType <= 28) { // IDA L1864
            Combat_UseElfSkill((int)(uintptr_t)entity, (int)(uintptr_t)entity);
            return AF_RETURN;                                 // IDA L1867
        }
        if (FUN_00483160() && iType == 51) {                  // IDA L1869: CheckAttack()
            if (*(WORD*)(entity + 2) == 390) {
                const WORD leftWeapon  = *(WORD*)(Hero + 624);   // IDA: v181
                const WORD rightWeapon = *(WORD*)(Hero + 648);   // IDA: v182
                if ((leftWeapon >= 536 && leftWeapon < 543)
                    || (rightWeapon >= 528 && rightWeapon < 535)
                    || rightWeapon == 545
                    || (leftWeapon >= 544 && leftWeapon <= 546))
                    Combat_UseElfSkill((int)(uintptr_t)entity, (int)(uintptr_t)entity);
            }
            return AF_LABEL_322;                              // IDA L1881
        }
    }
    if (iType < 26 || iType > 28)
        return AF_LABEL_322;                                  // IDA L1886
    // IDA L1917-2158: Heal / Greater Defense / Greater Damage sobre el heroe.
    Attack_SendSkill19_97k(iType, (WORD)DAT_05826cac);        // IDA: HeroKey
    FUN_00444a80((int)(uintptr_t)entity);                     // IDA LABEL_1430: SetPlayerMagic
    return AF_RETURN;
}

// ── IDA L1485-2893: rama ELF (clase 2) ───────────────────────────────────────
static Attack_Flow97k Attack_ElfBranch97k(char* entity, int iType)
{
    BYTE* const attr = (BYTE*)(uintptr_t)DAT_07cf1ff4;        // IDA: CharacterAttribute
    if (!attr || !Hero)
        return AF_RETURN;

    int piMana = 0, piSkillMana = 0;                          // IDA: piMana / piSkillMana
    GetSkillInformation(iType, 1, 0, &piMana, 0, &piSkillMana);   // IDA L1504
    if (piMana > (int)*(unsigned short*)(attr + 30)) {        // IDA L1505
        Attack_UseManaScroll97k();
        return AF_RETURN;                                     // IDA LABEL_142
    }
    if (piSkillMana > (int)*(unsigned short*)(attr + 36))     // IDA L1753
        return AF_RETURN;                                     // IDA LABEL_146

    CheckTarget_stub((DWORD)(uintptr_t)entity);               // IDA L1778 (retorno ignorado)

    if (iType >= 30 && iType <= 36) {
        // IDA L2617-2680: invocaciones del arbol Elf.
        if ((int)g_GameSubState == 10)                        // IDA L2617: World == 10
            return AF_RETURN;
        Attack_SendSkill19_97k(iType, (WORD)DAT_05826cac);    // IDA L2651-2655 + HeroKey
        FUN_00444a80((int)(uintptr_t)entity);                 // IDA LABEL_1430
        return AF_RETURN;
    }

    if (iType > 36) {                                         // IDA L2160
        // Solo 52 tiene rama propia; el resto cae en LABEL_240.
        if (iType == 52 && *(WORD*)(entity + 2) == 390) {     // IDA L2162
            const WORD leftWeapon  = *(WORD*)(Hero + 624);    // IDA: v83
            const WORD rightWeapon = *(WORD*)(Hero + 648);    // IDA: v84
            const bool weaponOk = (leftWeapon >= 536 && leftWeapon < 543)
                               || (rightWeapon >= 528 && rightWeapon < 535)
                               || rightWeapon == 545
                               || (leftWeapon >= 544 && leftWeapon <= 546);   // IDA L2167-2170
            if (weaponOk && Combat_CheckArrowRequirement()) { // IDA L2175: CheckArrow()
                const int range = Combat_GetSkillRange97k(52);   // IDA L2196
                if (!Attack_OutOfRange97k(entity, range)) {   // IDA L2203
                    *(float*)(entity + 36) = FUN_0043e050(    // IDA L2207: CreateAngle
                        *(float*)(entity + 16), *(float*)(entity + 20),
                        *(float*)(entity + 788), *(float*)(entity + 792));
                    DAT_05826d10 = 52;                        // IDA L2237: CurrentSkill
                    const BYTE dir = (BYTE)(int)(*(float*)(entity + 36) * 0.71111113f);
                    // IDA L2240-2290: [52][heroGX][heroGY][dir][0][0]
                    Combat_SendDuration1E_97k(entity, 52, dir, 0, 0);
                    FUN_00444410((int)(uintptr_t)entity, 0, 0, 0);   // IDA L2579: SetPlayerAttack
                    if (*(WORD*)(entity + 2) != 390) {        // IDA L2582
                        // IDA L2586-2601.  Hex-Rays muestra la clave del scan como
                        // `HIBYTE(v1025)` y el 6o argumento como `LOWORD(x1)` — dos
                        // slots de pila que reusa —, pero el disassembly (bytes
                        // leidos con ida_get_bytes desde 0x0049E640) los resuelve
                        // a los DOS al mismo byte:
                        //   0049E640  8A 16        mov dl, [esi]     ; esi = c+0x302 (770)
                        //   0049E648  88 54 24 3C  mov [esp+3Ch], dl ; -> base+0x38
                        //   0049E664  88 44 24 3B  mov [esp+3Bh], al ; -> base+0x37
                        //   0049E686  8A 4C 24 37  mov cl, [esp+37h] ; clave del scan
                        //   0049E6AD  8B 44 24 38  mov eax, [esp+38h]; arg6 = SKKey
                        //   0049E6B1  50 6A 00 56 6A 00 57 57 E8 ... call 0x00474BD0
                        // O sea SKKey es el byte de skill encolado `c + 770`.  Los 3
                        // bytes altos de ese slot son basura vieja y dan igual:
                        // CreateArrow (0x00474370 L83-91) solo usa `(_BYTE)SKKey`
                        // — lo compara contra 52 y 51 y lo guarda en CurrentSkill.
                        // El slot de la segunda mano NO viaja como argumento: la
                        // flecha/virote la elige CreateArrow por su cuenta con
                        // `pLeft = *(short *)(CharacterMachine + 604)` (L67).
                        const BYTE queuedSkill = (BYTE)entity[770];   // IDA: c + 0x302
                        int skillIndex = 0;                   // IDA: v128 / v129
                        while (skillIndex < 20 && attr[87 + skillIndex] != queuedSkill)
                            ++skillIndex;
                        CreateArrows_stub((DWORD)(uintptr_t)entity,
                                          (DWORD)(uintptr_t)entity, 0,
                                          (WORD)skillIndex, 0, (WORD)queuedSkill);
                    }
                }
            }
        }
        // IDA L2164/2172/2177/2205/2615: todos los caminos vuelven a LABEL_240.
    }

    return Attack_Label240_97k(entity, iType);
}

// ── IDA L2894-5920: LABEL_322, rama DK/MG (clases 1 y 3) ─────────────────────
// Sale por `return` en cada punto donde el binario hace `goto LABEL_932`; ese
// label termina en `break` (IDA L4702), o sea el flujo continua en L5921, que es
// justo lo que hace el caller al llamar despues a Attack_WizardBranch97k.
static void Attack_WarriorBranch97k(char* entity)
{
    const BYTE heroClass = (BYTE)(entity[444] & 7);           // IDA: v183
    if (heroClass != 1 && heroClass != 3)
        return;                                               // IDA L2896

    BYTE* const attr = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (!attr || !Hero)
        return;
    const int iType = attr[*(BYTE*)(Hero + 913) + 87];        // IDA L2915
    int piMana = 0, piSkillMana = 0;                          // IDA: v1031 / v1034
    GetSkillInformation(iType, 1, 0, &piMana, 0, &piSkillMana);   // IDA L2916
    if (*(WORD*)(entity + 2) != 390)
        return;                                               // IDA L2917

    // IDA L2921-2929: mano izquierda vacia, o BACULO en cualquier mano, deja
    // pasar UNICAMENTE el skill 48.  El rango 560..591 son model ids = tipo+400,
    // o sea tipos 160..191 = seccion 5 del item.bmd = Staff (verificado
    // decodificando Data/Local/item.bmd: 560 Skull Staff … 570 Divine Staff of
    // Archangel).  Las LANZAS son 496..527 (tipos 96..127, seccion 3), que es
    // justo el rango que exige Impale en Item_Equip.
    const WORD leftWeapon  = *(WORD*)(Hero + 624);            // IDA: v220
    const WORD rightWeapon = *(WORD*)(Hero + 648);            // IDA: v221
    if (leftWeapon == 0xFFFF
        || (leftWeapon >= 560 && leftWeapon < 592)
        || (rightWeapon >= 560 && rightWeapon < 592)) {
        if (iType != 48)
            return;
    }
    // IDA L2930-2937: v222 / HIBYTE(v1025) = "alcanza el AG del skill".
    const bool hasAg = (piSkillMana <= (int)*(unsigned short*)(attr + 36));
    const bool lowMana = (piMana > (int)*(unsigned short*)(attr + 30));

    switch (iType) {
    case 41: {                                                // IDA L2940
        *(float*)(entity + 36) = FUN_0043e050(
            *(float*)(entity + 16), *(float*)(entity + 20),
            *(float*)(entity + 788), *(float*)(entity + 792));
        if (lowMana) { Attack_UseManaScroll97k(); return; }
        if (!hasAg) return;                                   // IDA L3163
        DAT_05826d10 = (DWORD)iType;                          // IDA L3197
        Combat_SendDuration1E_97k(entity, iType, 0, 0, 0);    // IDA: [41][gx][gy][0][0][0]
        FUN_00443e70();                                       // IDA LABEL_497: SetAttackSpeed
        FUN_0043e820((int)(uintptr_t)entity, 61);
        entity[748] = 0;
        return;
    }
    case 42: {                                                // IDA L3537
        *(float*)(entity + 36) = FUN_0043e050(
            *(float*)(entity + 16), *(float*)(entity + 20),
            *(float*)(entity + 788), *(float*)(entity + 792));
        if (lowMana) { Attack_UseManaScroll97k(); return; }
        if (!hasAg) return;                                   // IDA L3762
        if (Attack_OutOfRange97k(entity, Combat_GetSkillRange97k(iType)))
        {
            // IDA L3771: el case 42 es el UNICO de los tres out-of-range de
            // esta rama que ademas apaga el auto-ataque.  Los cases 55 y 56
            // salen derecho a LABEL_932 sin tocar Attacking.
            DAT_00559c58 = -1;                                // IDA: Attacking = -1
            return;                                           // IDA L3772: goto LABEL_932
        }
        DAT_05826d10 = (DWORD)iType;                          // IDA L3809
        Combat_SendDuration1E_97k(entity, iType, 0, 0, 0);
        FUN_0043e820((int)(uintptr_t)entity, 62);             // IDA LABEL_930 (sin SetAttackSpeed)
        entity[748] = 0;
        return;
    }
    case 48: {                                                // IDA L4177 — Greater Life
        if (lowMana) { Attack_UseManaScroll97k(); return; }
        if (!hasAg) return;                                   // IDA L4397
        Attack_SendSkill19_97k(iType, (WORD)DAT_05826cac);    // IDA LABEL_771 + HeroKey
        FUN_0043e820((int)(uintptr_t)entity, 63);             // IDA LABEL_811
        entity[748] = 0;
        return;
    }
    case 55:                                                  // IDA L4665
    case 56: {                                                // IDA L5297
        *(float*)(entity + 36) = FUN_0043e050(
            *(float*)(entity + 16), *(float*)(entity + 20),
            *(float*)(entity + 788), *(float*)(entity + 792));
        if (lowMana) { Attack_UseManaScroll97k(); return; }
        if (!hasAg) return;                                   // IDA L4908 / L5522
        const BYTE packedOffset = Attack_PackDestination97k(entity);  // IDA L4914 / L5533, payload field 7
        if (Attack_OutOfRange97k(entity, Combat_GetSkillRange97k(iType)))
            return;                                           // IDA L4916-4919 / L5534-5537
        // IDA L5541 (solo el 56): payload field 8 = (angle + 180) / 360 * 255.
        const BYTE field8 = (iType == 56)
            ? (BYTE)(int)((*(float*)(entity + 36) + 180.0f) * 0.0027777778f * 255.0f)
            : (BYTE)0;
        DAT_05826d10 = (DWORD)iType;
        // IDA: [skill][heroGX][heroGY][0][packedOffset][field8].
        Combat_SendDuration1E_97k(entity, iType, 0, packedOffset, field8);
        FUN_00443e70();                                       // IDA LABEL_497 / LABEL_715
        FUN_0043e820((int)(uintptr_t)entity, (iType == 55) ? 61 : 81);
        entity[748] = 0;
        return;
    }
    default:
        return;                                               // IDA L5917: goto LABEL_932
    }
}

// ── IDA L7105-7776: case 16 del segundo switch (Soul Barrier) ────────────────
static void Attack_SoulBarrier97k(char* entity, int iType, bool hasTarget)
{
    if (SelectedCharacter != -1) {
        BYTE* const target = (BYTE*)(uintptr_t)CharactersClient +
                             916 * (int)SelectedCharacter;    // IDA: v541
        if (target[132] != 1) {                               // IDA L7109
            DAT_00559c58 = -1;                                // IDA L7111: Attacking = -1
            return;
        }
        if (PartyNumber <= 0)                                 // IDA L7115
            return;
        const char* const targetName = (const char*)(target + 449);   // IDA: v542
        size_t nameLen = strlen(targetName);
        if (nameLen == 0) nameLen = 1;
        bool inParty = false;
        for (int partyIndex = 0; partyIndex < PartyNumber; ++partyIndex) {   // IDA L7120-7131
            if (strncmp((const char*)Party + partyIndex * 36, targetName, nameLen) == 0) {
                inParty = true;
                break;
            }
        }
        if (!inParty)
            return;
        *(WORD*)(entity + 784) = (WORD)SelectedCharacter;     // IDA L7133
        Attack_SeedSkillTarget97k();                          // IDA L7134-7141
        const int range = Combat_GetSkillRange97k(iType);     // IDA L7170
        if (Attack_OutOfRange97k(entity, range)) {            // IDA L7173
            if (SelectedCharacter != -1) {
                if (Path_FindRoute(*(int*)(entity + 904), *(int*)(entity + 908),
                                   (int)DAT_07e016c0, (int)DAT_07e016c4,
                                   (unsigned char*)(entity + 852), (float)range)) {
                    entity[748] = 1;                          // IDA L7205
                    entity[749] = 5;                          // IDA L7206
                }
            }
            return;                                           // IDA L7212
        }
        // IDA L7264-7506: C1:06:19 con la key del objetivo, y despues LABEL_1157.
        const WORD targetKey = *(WORD*)((BYTE*)(uintptr_t)CharactersClient +
                                        916 * (int)DAT_07d780a0 + 476);
        Attack_SendSkill19_97k(iType, targetKey);
    } else {
        // IDA L7508-7754: sin objetivo el skill se lanza sobre uno mismo.
        Attack_SendSkill19_97k(iType, (WORD)DAT_05826cac);    // IDA: HeroKey
    }
    FUN_00444a80((int)(uintptr_t)entity);                     // IDA LABEL_1157: SetPlayerMagic
    Attack_Label1158_97k(entity, iType, hasTarget);           // IDA: cae en LABEL_1158
}

// ── IDA L7779-7960: LABEL_1158 + LABEL_1354 ──────────────────────────────────
static void Attack_Label1158_97k(char* entity, int iType, bool hasTarget)
{
    Attack_SeedSkillTarget97k();                              // IDA L7780-7788
    const int range = Combat_GetSkillRange97k(iType);         // IDA L7816
    if (Attack_OutOfRange97k(entity, range)) {                // IDA L7819
        if (SelectedCharacter != -1 && FUN_00483160()) {      // IDA L7821: CheckAttack()
            if (Path_FindRoute(*(int*)(entity + 904), *(int*)(entity + 908),
                               (int)DAT_07e016c0, (int)DAT_07e016c4,
                               (unsigned char*)(entity + 852), (float)range)) {
                entity[748] = 1;                              // IDA L7864
                entity[749] = 5;                              // IDA L7865
            }
        }
        return;                                               // IDA L7871
    }
    if (!Path_IsLineClear(*(int*)(entity + 904), *(int*)(entity + 908),
                          (int)DAT_07e016c0, (int)DAT_07e016c4)) {   // IDA L7930
        Attack_Label1585_97k(entity, iType, hasTarget);       // IDA LABEL_1584 -> LABEL_1585
        return;
    }
    if (SelectedCharacter != -1 && FUN_00483160()) {          // IDA L7938-7940
        Combat_UseWizardSkill((DWORD)(uintptr_t)entity,
                              (DWORD)(uintptr_t)entity);      // IDA L7943: sub_4889D0
    }
    // IDA LABEL_1354 (L7950).
    if (hasTarget) {                                          // IDA L7951: HIBYTE(v1025)
        *(float*)(entity + 36) = FUN_0043e050(                // IDA L7953: CreateAngle
            *(float*)(entity + 16), *(float*)(entity + 20),
            *(float*)(entity + 788), *(float*)(entity + 792));
        const BYTE dir = (BYTE)(int)(*(float*)(entity + 36) * 0.71111113f);
        switch (iType) {
        case 5:                                               // IDA L7962 — Flame
            DAT_05826d10 = (DWORD)iType;                      // IDA LABEL_1521
            // IDA L8035/L8072: la posicion viene de c+788/792 / 100, no de la grilla.
            Combat_SendDuration1E_97k(entity, iType, dir, 0, 0,
                                      (int)(*(float*)(entity + 788) * 0.01f),
                                      (int)(*(float*)(entity + 792) * 0.01f));
            FUN_00444a80((int)(uintptr_t)entity);             // IDA LABEL_1430
            return;
        case 8:                                               // IDA L8369
        case 9:
            DAT_05826d10 = (DWORD)iType;
            Combat_SendDuration1E_97k(entity, iType, dir, 0, 0);
            FUN_00444a80((int)(uintptr_t)entity);             // IDA LABEL_1430
            return;
        case 12:                                              // IDA L8800 — Aqua Beam
            if (*(WORD*)(entity + 2) != 390)
                return;                                       // IDA L8801-8804
            DAT_05826d10 = (DWORD)iType;                      // IDA LABEL_1441
            Combat_SendDuration1E_97k(entity, iType, dir, 0, 0);
            FUN_00443e70();                                   // IDA LABEL_1512
            FUN_0043e820((int)(uintptr_t)entity, 88);
            entity[748] = 0;
            DAT_07e11db8 = 0;                                 // IDA L9208
            return;
        default:
            break;                                            // IDA L9258: goto LABEL_1584
        }
    }
    Attack_Label1585_97k(entity, iType, hasTarget);
}

// ── IDA L9261-10099: LABEL_1585 (cola exclusiva del Dark Wizard) ─────────────
static void Attack_Label1585_97k(char* entity, int iType, bool hasTarget)
{
    if ((entity[444] & 7) != 0 || !hasTarget)
        return;                                               // IDA L9262

    if (iType == 6) {                                         // IDA L9266 — Teleport
        if (DAT_07e91388 > 0)                                 // IDA L9268: pPickedItem
            return;
        const BYTE currentAction = (BYTE)entity[261];         // IDA: v946
        if (*(WORD*)(entity + 2) == 390) {
            if (currentAction == 87) return;                  // IDA L9275
        } else if (currentAction == 5) {
            return;                                           // IDA L9280
        }
        const BYTE fadeState = (BYTE)entity[124];             // IDA: v947
        if (fadeState == 1 || fadeState == 2 || *(float*)(entity + 360) < 0.69999999f)
            return;                                           // IDA L9285
        extern unsigned char* TerrainWall;
        BYTE terrainAttr = TerrainWall[(((unsigned)DAT_07e016c4 & 0xFF) << 8)
                                       | ((unsigned)DAT_07e016c0 & 0xFF)];   // IDA: TerrainWall[Terrain_Load(TargetX, TargetY)]
        if ((terrainAttr & 0x20) == 0x20)
            terrainAttr -= 0x20;                              // IDA L9290-9292
        if (terrainAttr != 0)
            return;                                           // IDA L9294
        *(float*)(entity + 36) = FUN_0043e050(                // IDA L9298
            *(float*)(entity + 16), *(float*)(entity + 20),
            *(float*)(entity + 788), *(float*)(entity + 792));
        // IDA L9303: Teleport || dword_7E11DC4 || GetTickCount() - dword_7E11DC8 < 3000.
        // Los tres globals existen en nuestro arbol y ya los escribe el handler
        // 0x1C de Net_Process (ReceiveTeleport), asi que el cooldown queda
        // compartido igual que en el binario.
        if (DAT_05826d04 || DAT_07e11dc4 || (GetTickCount() - DAT_07e11dc8) < 3000)
            return;
        DAT_05826d04 = 1;                                     // IDA L9307: Teleport = 1
        // IDA Attack @0x49CBF0 arma el buffer con longitud inicial 3 y luego
        // anexa gate=0, TargetX y TargetY: [C1][06][1C][00][TargetX][TargetY].
        // El `05` que aparecia en una lectura anterior era el valor intermedio
        // previo al ultimo append, no la longitud enviada.
        BYTE packet[6] = { 0xC1, 0x06, 0x1C, 0x00,
                           (BYTE)DAT_07e016c0, (BYTE)DAT_07e016c4 };
        Net_SendSmallPacket(packet, sizeof(packet));
        // IDA L9682 LABEL_1762 -> LABEL_1763: sub_444B30(c) = SetPlayerTeleport,
        // o sea la animacion de casteo (accion 87) sobre el propio heroe.
        // MU 5.2 ZzzInterface.cpp:6130 hace lo mismo tras SendRequestMagicTeleport.
        FUN_00444b30((int)(uintptr_t)entity);
        return;
    }

    if (iType == 15 && SelectedCharacter != -1) {             // IDA L9737
        char* const target = (char*)(uintptr_t)CharactersClient + 916 * (int)SelectedCharacter;
        if (PartyNumber <= 0 || DAT_07e91388 > 0) {   // IDA L9740
            return;
            }
        const char* const targetName = target + 449;
        size_t nameLen = strlen(targetName);
        if (nameLen == 0) nameLen = 1;
        bool inParty = false;
        for (int partyIndex = 0; partyIndex < PartyNumber; ++partyIndex) {
            if (strncmp((const char*)Party + partyIndex * 36, targetName, nameLen) == 0) {
                inParty = true;
                break;
            }
        }
        if (!inParty) {
            return;
            }
        const bool targetIsPlayer = (*(unsigned short*)(target + 2) == 390);
        const BYTE targetAction = *(unsigned char*)(target + 261);
        if ((targetIsPlayer && targetAction == 87) || (!targetIsPlayer && targetAction == 5)) {
            return;
            }
        const BYTE targetFade = *(unsigned char*)(target + 124);
        if (targetFade == 1 || targetFade == 2 || *(float*)(target + 360) < 0.7f) {
            return;
            }
        extern unsigned char* TerrainWall;
        const int heroGX = (int)(*(float*)(entity + 16) * 0.01f);
        const int heroGY = (int)(*(float*)(entity + 20) * 0.01f);
        for (int attempt = 0; attempt <= 10; ++attempt) {       // IDA L9781-9795
            const int rx = rand() % 3;                          // IDA: v909
            const int ry = rand() % 3;                          // IDA: v910
            if (rx == 1 && ry == 1) continue;                   // IDA L9784
            const int x = heroGX + rx - 1;
            const int y = heroGY + ry - 1;
            DAT_07e016c0 = (DWORD)x;                            // IDA L9786: TargetX
            DAT_07e016c4 = (DWORD)y;                            // IDA L9787: TargetY
            BYTE terrainAttr = TerrainWall[((y & 0xFF) << 8) | (x & 0xFF)];   // IDA: TERRAIN_INDEX
            if ((terrainAttr & 0x20) == 0x20) terrainAttr -= 0x20;
            *(float*)(target + 36) = FUN_0043e050(              // IDA L9798
                *(float*)(target + 16), *(float*)(target + 20),
                *(float*)(target + 788), *(float*)(target + 792));
            if (DAT_05826d04) {                                 // IDA L9802: if (Teleport)
                DAT_05826d04 = 0;
                return;
            }
            DAT_05826d04 = 1;                                   // IDA L9807: Teleport = 1
            Combat_SeedRuntimeState97k(iType, (int)SelectedCharacter);
            Combat_SendPartyRecall97k(entity, (int)SelectedCharacter, (BYTE)x, (BYTE)y);
            // IDA L10084 LABEL_1670 -> LABEL_1763: sub_444B30 = SetPlayerTeleport.
            // Hex-Rays muestra `v945 = iType` justo antes del label, pero ese slot
            // esta aliaseado (la funcion tiene ~1200 locales) y sub_444B30 deferencia
            // o+2: un iType chico seria un puntero salvaje. MU 5.2 desambigua el
            // argumento (ZzzInterface.cpp:6065): en el recall de party la animacion
            // va sobre el ALIADO (`SetPlayerTeleport(tc)`), no sobre el heroe.
            FUN_00444b30((int)(uintptr_t)target);
            return;
        }
    }
}

// ── IDA L5921-6246 + switch L6247: rama DW/MG ────────────────────────────────
static void Attack_WizardBranch97k(char* entity)
{
    const BYTE heroClass = (BYTE)(entity[444] & 7);
    if (heroClass != 0 && heroClass != 3)
        return;                                               // IDA L5921

    BYTE* const attr = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (!attr || !Hero)
        return;
    const int iType = attr[*(BYTE*)(Hero + 913) + 87];        // IDA L5950
    int piMana = 0, piSkillMana = 0;                          // IDA: v1035 / v1033
    GetSkillInformation(iType, 1, 0, &piMana, 0, &piSkillMana);   // IDA L5951
    if (piMana > (int)*(unsigned short*)(attr + 30)) {        // IDA L5952
        Attack_UseManaScroll97k();
        return;                                               // IDA LABEL_999
    }
    if (piSkillMana > (int)*(unsigned short*)(attr + 36))     // IDA L6212
        return;

    // IDA L6246: `HIBYTE(v1025) = CheckTarget(c)` — CheckTarget devuelve bool.
    const bool hasTarget = (CheckTarget_stub((DWORD)(uintptr_t)entity) != 0);

    switch (iType) {                                          // IDA L6247
    case 10:                                                  // IDA L6249 — Hell Fire
    case 14:                                                  // IDA L6671 — Inferno
        if (*(WORD*)(entity + 2) != 390)
            return;
        DAT_05826d10 = (DWORD)iType;                          // IDA LABEL_1172 / LABEL_1245
        // [iType][heroGX][heroGY][0][0][0] — sin direccion ni delta.
        Combat_SendDuration1E_97k(entity, iType, 0, 0, 0);
        FUN_00443e70();                                       // IDA LABEL_1235 / LABEL_1310
        FUN_0043e820((int)(uintptr_t)entity, (iType == 10) ? 90 : 89);
        entity[748] = 0;
        return;
    case 16:                                                  // IDA L7105
        Attack_SoulBarrier97k(entity, iType, hasTarget);
        return;
    case 18: case 19: case 20: case 21:                       // IDA L10100-10108
    case 22: case 23: case 55: case 56:
        return;
    default:                                                  // IDA L10109
        Attack_Label1158_97k(entity, iType, hasTarget);
        return;
    }
}

void __cdecl Combat_DispatchHeroSkillAttack(void *entity_v /* IDA: c */)
{
    char* entity = (char*)entity_v;   // IDA: c
    if (!entity) return;

    // 1) Early exits — anti-tamper / loading state ──────────────────────────────────────────────────────────────────────────
    if (DAT_07e11d30 != 0) return;          // IDA L1236: EditFlag (0x07E11D30)

    // 2) Stamina decay on entity (movement-tick interpolation) ────────────────────────────────────────────────
    // IDA L1240-1247.  c+124 es el estado de fade y c+360 (0x168) el Alpha:
    // `v5 = *(float *)(c + 360) - 0.1; ... if (v5 < 0.1) c[124] = 2;`.
    // El titulo "stamina" de este bloque era una etiqueta inventada del port.
    if (entity[0x7c] == 1) {                      // IDA: *(_BYTE *)(c + 124)
        float& fAnim = *(float*)(entity + 0x168); // IDA: v5 = *(float *)(c + 360) — Alpha
        fAnim -= 0.1f;                       // IDA: -= 0.1
        if (fAnim < 0.1f) entity[0x7c] = 2;
    }

    // 3) Mouse button gate ──────────────────────────────────────────────────────────────────────────
    bool bSuccess = false;                   // IDA: Success
    if (DAT_07e11dc0 != 0) {                 // IDA: byte_7E11DC0
        // Movimiento bloqueado: sólo seguir si se apretó el botón izquierdo
        if (DAT_083a4124 == 0 && DAT_083a42c4 == 0) return;
        DAT_083a4124 = 0;  // MouseLButtonPush = 0
    } else {
        // Normal: RMB triggers attack
        if (DAT_083a42d0 != 0 || DAT_083a42ac != 0) {
            DAT_083a42d0 = 0;
            bSuccess = true;
        }
        // 0049CBF0: la continuación automática sólo vale para el
        // Attacking==2 state.  Omitting that state turns any stale selected
        // entidad en un casteo perpetuo después de soltar el botón derecho.
        const int worldIndex = (int)g_GameSubState;
        const bool autoAttackOK = (DAT_07e11e18 != 0)         // m_bAutoAttack
            && worldIndex != 6                                // not in dialog/loading
            && DAT_00559c58 == 2                              // IDA: Attacking (0x00559C58)
            && SelectedCharacter != -1;                            // IDA: SelectedCharacter (0x00559C50)
        if (!autoAttackOK && !bSuccess) {
            g_RightButtonPressSeconds_Attack = 0.0f;
            g_RightButtonReleaseTime_Attack = DAT_05826e08;
            return;
        }
        // IDA: (WorldTime - RButtonPopTime) * 0.001; release both latches
        // once the hold reaches 3600 seconds.
        // IDA L1272-1273: v7 = (WorldTime - RButtonPopTime) * 0.001; RButtonPressTime = v7;
        g_RightButtonPressSeconds_Attack =
            (DAT_05826e08 - g_RightButtonReleaseTime_Attack) * 0.001f;
        if (g_RightButtonPressSeconds_Attack >= 3600.0f) {
            DAT_083a42d0 = 0;
            DAT_083a42ac = 0;
            return;
        }
    }

    // 4) Death / SafeZone gate (IDA Attack L1280) —
    //      if (c[765] || c[846] && (World < 11 || World > 16)) return;
    // +765 (0x2FD) = dead_flag real (lo setea ReceiveDie).
    // +846 (0x34E) = **SafeZone**, NO "mount-only": vale
    //      TerrainWall[Terrain_Load(x,y)] & 1.  La etiqueta vieja venia de la
    //      tabla de offsets de CLAUDE.md, que estaba mal (ver la entrada
    //      "+0x34E es SafeZone, no dead_flag", 2026-08-10).  La logica ya era
    //      correcta; solo el comentario mentia.
    // O sea: muerto, o parado en zona segura fuera de los mapas 11..16.
    if (entity[765] != 0) return;
    if (entity[846] != 0) {
        const int W = (int)g_GameSubState;
        if (W < 11 || W > 16) return;
    }

    // 5) Resolve iType = equipped skill at slot Hero[+913] ────────────────────────────────────────────────────────────
    // CharacterAttribute is a byte array; offset = Hero[913] + 87 — skill ID byte.
    // IDA L1303: v13 = *(BYTE *)(*(BYTE *)(Hero + 913) + CharacterAttribute + 87)
    int iType = 0;                            // IDA: iType / v13
    if (Hero != 0) {
        iType = (int)Combat_ResolveQueuedSkillId97k();
    }

    // 6) Segundo gate de SafeZone (IDA Attack L1314): dentro del pueblo solo
    //    pasan los skills 26/27/28 (buffs de Elf), 16 y 48 (Greater Life).
    if (entity[846] != 0 && iType != 26 && iType != 27 && iType != 28
        && iType != 16 && iType != 48) {
        return;
    }

    // 7) 0049CBF0 escribe Attacking desde el estado de movimiento de la entidad. Ése es
    // el estado que consume el gate de auto-ataque de la invocación siguiente, arriba.
    // 2026-09-01 FIX — global partido en dos.  Esto escribia `DAT_07e11984`,
    // que en globals.h es el *debounce de la flecha arriba del chat* (lo escribe
    // Chat_InputTick con 0/1).  El `Attacking` de IDA vive en **0x00559C58**:
    // verificado con ida_xrefs_to — lo escriben InitGame L38 (=-1),
    // Player_InputTick L942 (=1) y este Attack (=2/-1), y lo leen el gate de
    // auto-ataque de arriba y Player_InputTick L599.  En nuestro arbol esa
    // direccion es `DAT_00559c58`, que Mouse_Hover ya usa con esa semantica.
    // Con el global equivocado el gate `Attacking == 2` no se cumplia nunca y
    // la continuacion de auto-ataque quedaba muerta.
    if (DAT_07e11e18 != 0 && (int)g_GameSubState != 6) {  // IDA: m_bAutoAttack, World
        const BYTE movementState = entity[444] & 7;       // IDA: v16 = c[444] & 7 (clase)
        if ((movementState != 2 || iType == 24 || iType == 25 || iType == 52)
            && (movementState != 1 || iType != 48)) {
            DAT_00559c58 = 2;      // IDA: Attacking = 2
        } else {
            DAT_00559c58 = -1;     // IDA: Attacking = -1
        }
    }

    // 8) MouseOnWindow (IDA Attack L1330) — si el cursor esta sobre una ventana
    // de UI, no se ataca.  2026-08-15: estaba diferido ("no trackeamos ese
    // global"), pero SI existe: `g_MouseOnWindow`, que puebla
    // `MouseOnWindow_Update` en Player_InputTick.cpp (y al que el widget de chat
    // le pasa su latch `g_ChatLB_MouseOnWindow`).  Sin este gate, click derecho
    // sobre el inventario / chat / paneles disparaba el skill igual.
    if (g_MouseOnWindow) return;             // IDA L1330: MouseOnWindow (0x07D78094)

    // 8b) Gate por estado de animacion (IDA Attack L1335-1345).  `c+261` es el
    // action-state; el heroe (entity_type 390) solo puede atacar desde los
    // estados 1..12 y el 78, y cualquier otra entidad solo desde 0 o 1.  Sin
    // esto se podia relanzar el skill durante la animacion de recuperacion.
    {
        const BYTE actionState = (BYTE)entity[261];   // IDA: v17 = *(_BYTE *)(c + 261)
        if (*(short*)(entity + 2) == 390) {
            if (actionState == 0 || (actionState > 0x0C && actionState != 78))
                return;
        } else if (actionState > 1) {
            return;
        }
    }

    // 9) Chequeo anti-cheat de strings (aWebzen_*) — salteado por policy del
    //     proyecto.  IDA lo usa SOLO como gate para saltar el envio del paquete;
    //     no cambia estado ni el flujo posterior (todos sus destinos convergen en
    //     el mismo label que la rama que si envia), asi que omitirlo no altera
    //     condiciones ni side effects.  Verificado en los 12 sitios de Attack.

    // ══════════════════════════════════════════════════════════════════════════
    // IDA L1346-1484 — bucle de manos equipadas, comun a las clases != 0.
    // ══════════════════════════════════════════════════════════════════════════
    if ((entity[444] & 7) != 0) {
        CheckTarget_stub((DWORD)(uintptr_t)entity);            // IDA L1348
        const int heroGridY = *(int*)(entity + 908);           // IDA: v18
        const int heroGridX = *(int*)(entity + 904);           // IDA: v19
        // IDA L1355: CheckWall(v19, v18, TargetX, TargetY) — 0x004830B0, que en
        // nuestro arbol se llama Path_IsLineClear (functions.h lo mapea a esa
        // misma direccion).  No es una aproximacion: es la misma funcion.
        if (Path_IsLineClear(heroGridX, heroGridY,
                             (int)DAT_07e016c0, (int)DAT_07e016c4)) {
            for (int i = 0; i <= 68; i += 68) {                // IDA L1357: i
                const BYTE handClass = (BYTE)(entity[444] & 7);   // IDA: v21
                if (handClass == 1 || handClass == 3) {
                    const WORD helper = *(WORD*)(entity + 696); // IDA: v22
                    bool handAllowed = true;                    // IDA: v23
                    if (helper == 818 || helper == 819) {
                        // Montado: solo 43/47/49 (IDA L1385-1390).
                        handAllowed = false;
                        BYTE* const attr = (BYTE*)(uintptr_t)DAT_07cf1ff4;
                        if (attr && Hero) {
                            const BYTE sk = attr[*(BYTE*)(Hero + 913) + 87];
                            handAllowed = (sk == 43 || sk == 47 || sk == 49);
                        }
                    }
                    if (handAllowed) {
                        DAT_07d78098 = 1;                       // IDA L1422
                        DAT_07d780a0 = (DWORD)SelectedCharacter;// IDA L1424
                        DAT_07d7809c = Hero ? *(BYTE*)(Hero + 913) : 0; // IDA L1423/1425
                        if (DAT_07cf1ffc != 0 &&
                            Item_Equip((DWORD)(uintptr_t)entity,
                                       (DWORD)((uintptr_t)DAT_07cf1ffc + 536 + i)))
                            return;                             // IDA L1431: LABEL_146
                    }
                }
                if ((entity[444] & 7) == 2) {                   // IDA L1445
                    if (DAT_07cf1ffc != 0 &&
                        Combat_UseElfSkillItem((DWORD)(uintptr_t)entity,
                                               (DWORD)((uintptr_t)DAT_07cf1ffc + 536 + i)))
                        return;                                 // IDA L1469: LABEL_146
                }
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // IDA L1485-2893 — rama ELF (clase 2).  El gate esta en el disassembly:
    //   0049D1EB  mov cl, [ebp+1BCh] / and cl,7 / cmp cl,2 / jnz 0x0049F195
    // o sea el chequeo de mana + scroll y todo el dispatch de skills bajos de
    // este bloque son EXCLUSIVOS de la Elf.  Las otras clases saltan a LABEL_322.
    // ══════════════════════════════════════════════════════════════════════════
    if ((entity[444] & 7) == 2) {
        if (Attack_ElfBranch97k(entity, iType) == AF_RETURN)
            return;
        // AF_LABEL_322: los `goto LABEL_322` de L1881/L1886 caen aca.
    }

    // ══════════════════════════════════════════════════════════════════════════
    // IDA L2894 LABEL_322 — rama DK/MG (clases 1 y 3).
    // ══════════════════════════════════════════════════════════════════════════
    Attack_WarriorBranch97k(entity);

    // ══════════════════════════════════════════════════════════════════════════
    // IDA L5921 — desde aca solo siguen DW (0) y MG (3).
    // ══════════════════════════════════════════════════════════════════════════
    Attack_WizardBranch97k(entity);
}

// IDA: FUN_0048ba70 @ 0x0048BA70 — CheckArrow(void)
// Validates the bow/bolt pairing held by CharacterMachine.  The two item
// types are at +536 and +604; the byte flags used by the exceptional pairs
// are +562 and +630.
// All anti-tamper hash table operations (MAIN_HASH_CLASS) skipped per project policy.
char __cdecl Combat_CheckArrowRequirement(void)
{
    // anti-tamper hash table — skipped
    BYTE* pMachine = (BYTE*)DAT_07cf1ffc;
    if (!pMachine) return 0;

    const short leftType = *(short*)(pMachine + 536);
    const short rightType = *(short*)(pMachine + 604);

    if ((leftType >= 136 && leftType < 143) || leftType == 144 || leftType == 146) {
        if (rightType == 135 && pMachine[630]) return 1;
        UIChatLogWindow_AddText((const char*)&DAT_07e11df4, GlobalText[251], 2);
        FUN_0048b680(rightType);
        return 0;
    }
    if ((rightType < 128 || rightType >= 135) && rightType != 145) return 1;
    if (leftType == 143 && pMachine[562]) return 1;
    UIChatLogWindow_AddText((const char*)&DAT_07e11df8, GlobalText[251], 2);
    FUN_0048b680(rightType);
    return 0;
}

// ──────────────────────────────────────────────────────────────────────────
// LEGACY STUB — Send_MovePacket_Player_legacy_stub(void)
//
// Originalmente este stub vivía en la dirección 0x0048D640 con el nombre
// "Send_MovePacket_Player". El IDA companion confirma que la función real en
// 0x0048D640 se llama "Action" (despachador de acciones — pickup/equip/attack/
// skill — basado en `*(c+749)` queue), NO un sender de packet 0x10.
//
// Sin embargo, los call-sites existentes de Combat_ProcessQueuedAction() en nuestro codebase
// (5 en Player_InputTick, 1 en stubs/UseSkillWarrior) fueron escritos contra
// este stub y dependen de su comportamiento de "enviar packet 0x10 cuando el
// pathfind falla / cancela". Cambiar Combat_ProcessQueuedAction al Action real rompería
// el envío de move-packets en runtime.
//
// Estrategia: este stub mantiene su comportamiento original (renombrado para
// que callers apunten a él explícitamente). Combat_ProcessQueuedAction abajo es ahora el
// Action real con signature (DWORD c, DWORD o) tomada de IDA.
//
// Comportamiento (sin cambios respecto del stub original):
//  - Lee waypoints/facing/target_grid del player entity (DAT_07abf5d8).
//  - Construye [0xC1][len][0x10][...] con XOR-encoded payload (key login).
//  - Manda directo via send() con WSAEWOULDBLOCK queue fallback.
//  - Phantom stack params anti-tamper skipped.
// ──────────────────────────────────────────────────────────────────────────
void __cdecl Send_MovePacket_Player_legacy_stub(void)
{
    // Delega en el emisor de paquetes de movimiento a nivel entidad, usando la entidad del jugador
    char* entity = DAT_07abf5d8; // player entity base
    if (entity == nullptr) return;

    // Punto de entrada de compatibilidad: usa el emisor C1 verificado. La copia local
    // vieja ponía el opcode 0x10 en el byte 3 y después envolvía el paquete como C3.
    Combat_SendMovePathPacket((int)(intptr_t)entity, (int)(intptr_t)entity);
    return;

    unsigned char wpCount = *(unsigned char*)(entity + 0x356);
    if (wpCount == 0) return;
    if (wpCount > 0xe) wpCount = 0xe;

    // DAT_00559bec = packet length code
    if (wpCount < 3)
        DAT_00559bec = 0;
    else if (wpCount == 3)
        DAT_00559bec = 5;
    else
        DAT_00559bec = (unsigned int)wpCount * 3 + 4;

    // Clave XOR (la misma que Combat_SendMovePathPacket / el paquete de login)
    static const unsigned char xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };

    unsigned char pkt[64];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0xC1;
    pkt[2] = 0x00;
    pkt[3] = 0x10; // move opcode

    pkt[4] = *(unsigned char*)(entity + 0x306); // target_x
    pkt[5] = *(unsigned char*)(entity + 0x307); // target_y

    float facing = *(float*)(entity + 0x24);
    pkt[6] = (unsigned char)((int)(facing / 360.0f * 256.0f) & 0xFF);
    pkt[7] = wpCount;

    for (int i = 0; i < wpCount; i++)
        pkt[8 + i]          = *(unsigned char*)(entity + 0x357 + i);
    for (int i = 0; i < wpCount; i++)
        pkt[8 + wpCount + i] = *(unsigned char*)(entity + 0x366 + i);

    unsigned int pktLen = 8 + wpCount * 2;
    pkt[1] = (unsigned char)pktLen;

    // XOR-encode from byte index 3 onward
    for (unsigned int i = 3; i < pktLen; i++) {
        unsigned int ki = i & 0x1f;
        pkt[i] ^= xorKey[ki] ^ pkt[i - 1];
    }

    Net_SendSmallPacket(pkt, (int)pktLen);
}

static BYTE Combat_ResolveQueuedSkillId97k()
{
    // Attack @ 0049CBF0 no usa acá el slot cacheado de MovementSkillTarget.
    // Siempre deferencia el slot equipado actual en Hero+913:
    //   *(BYTE *)(*(BYTE *)(Hero + 913) + CharacterAttribute + 87)
    // Using DAT_07d7809c when no queued move existed made a regular RMB cast
    // resuelven al skill 0 y retornan antes de cualquier animación o envío de paquete.
    BYTE* charAttr = (BYTE*)DAT_07cf1ff4;
    if (!Hero || !charAttr) return 0;
    return charAttr[*(BYTE*)(Hero + 913) + 87];
}

static int Combat_GetSkillRange97k(int skillType)
{
    if (skillType < 0 || skillType >= 64)
        return 0;

    // GetSkillInformation @ 47E7A0 takes this from clear SkillAttribute
    // byte +38.  The encrypted shadow's byte +39 is unrelated metadata.
    return (int)SkillAttribute.Raw[skillType * 0x28 + 0x26];
}

static void Combat_SeedRuntimeState97k(int skillType, int targetIdx)
{
    DAT_07d78098 = 1;
    DAT_07d7809c = (Hero ? *(unsigned char*)(Hero + 913) : 0);
    DAT_07d780a0 = (DWORD)targetIdx;
    DAT_05826d10 = (DWORD)skillType;
}

static void Combat_SendPartyRecall97k(char* entity, int targetIdx,
                                      BYTE destinationX, BYTE destinationY)
{
    if (!entity || targetIdx < 0) return;
    DWORD selOff = 916 * targetIdx;
    unsigned short tgtKey = *(unsigned short*)(CharactersClient + selOff + 476);
    unsigned char pkt[7];
    pkt[0] = 0xC1;
    pkt[1] = 7;
    pkt[2] = 0xB0;
    // PMSG_SKILL_TELEPORT_ALLY_RECV guarda el índice como un WORD empaquetado, así que la
    // escritura nativa del 0.97k en v1077+3 es little-endian. Esto difiere de los
    // campos de objetivo big-endian de dos bytes que usan C3:19 y C3:1E.
    pkt[3] = (unsigned char)(tgtKey & 0xFF);
    pkt[4] = (unsigned char)((tgtKey >> 8) & 0xFF);
    pkt[5] = destinationX;
    pkt[6] = destinationY;
    Net_SendSmallPacket(pkt, 7);
}

// Constructor nativo del 0.97k: C1:09:1E con seis bytes de payload
//   [skill][x][y][field6][field7][field8].  No lleva la key del objetivo:
// los cases 41/42/48/55/56 y 5/8/9/10/12/14 de 0x49CBF0 anexan exactamente seis
// bytes y el `if (v1055[2] == 193) v1055[3] = v1055[0]` deja el tamanio en 9.
static void Combat_SendDuration1E_97k(char* entity, int skillType,
                                      BYTE field6, BYTE field7, BYTE field8,
                                      int packetX, int packetY)
{
    // DESVIACION DE PROTOCOLO (servidor MuEmu), documentada:
    //
    // El 0.97k vanilla manda 6 bytes de payload (tamanio 9) y NO manda la key
    // del objetivo.  MuEmu espera PMSG_DURATION_SKILL_ATTACK_RECV
    // (GameServer/SkillManager.h:96), que son 11 bytes:
    //     [C3][0B][1E][skill][x][y][dir][dis][angle][index0][index1]
    // y lee `index[]` SIEMPRE:
    //     short bIndex = MAKE_NUMBERW(lpMsg->index[0], lpMsg->index[1]);
    //     this->UseDurationSkillAttack(..., bIndex, ...);   // SkillManager.cpp:2045
    // Con un paquete de 9 bytes el server lee esos dos bytes FUERA del paquete:
    // `bIndex` sale basura y `MultiSkillAttack -> BasicSkillAttack(aIndex,
    // bIndex, ...)` le pega a otra entidad — el sintoma de "se muere otro
    // monstruo en otra posicion", y Penetration directamente no hace nada.
    //
    // Es lo mismo que hace el DLL de inyeccion para que un cliente 0.97k
    // funcione contra este server: CPatchs::SendRequestMagicContinue
    // (Source/Client/Main/Patchs.cpp:2012) arma los 11 bytes y los 13 hooks
    // SendContinue* pasan SIEMPRE
    //     index = CheckAttack() ? *(short *)(CharactersClient + 916*Sel + 0x1DC)
    //                           : -1;
    const int targetIndex = Combat_SkillTargetIndex97k();

    BYTE pkt[11];
    pkt[0] = 0xC1;
    pkt[1] = 11;
    pkt[2] = 0x1E;
    pkt[3] = (BYTE)skillType;
    pkt[4] = (BYTE)((packetX >= 0) ? packetX : *(DWORD*)(entity + 904));
    pkt[5] = (BYTE)((packetY >= 0) ? packetY : *(DWORD*)(entity + 908));
    pkt[6] = field6;                              // DLL: dir
    pkt[7] = field7;                              // DLL: dis
    pkt[8] = field8;                              // DLL: angle
    pkt[9]  = (BYTE)((targetIndex >> 8) & 0xFF);  // DLL: HIBYTE(target)
    pkt[10] = (BYTE)(targetIndex & 0xFF);         // DLL: LOBYTE(target)
    Net_SendSmallPacket(pkt, sizeof(pkt));
}

static void Combat_SendPlainPacket97k(BYTE* pkt, int len)
{
    static const BYTE xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };
    if (!pkt || len < 3) return;
    for (int i = 3; i < len; ++i) {
        pkt[i] ^= pkt[i - 1] ^ xorKey[i & 0x1F];
    }
    Net_SendBuf((const char*)pkt, len);
}

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
//   SetPlayerAttack         FUN_00444410 — local attack anim dispatcher
//   PathFinding2            Path_FindRoute — pathfind helper (returns 0/1)
//   Movement_Tick           FUN_0043e050 — atan2-based facing-toward (CreateAngle)
//   CheckWall               Path_IsLineClear — line-of-sight tile check (PathRange_Check)
//   UseSkillWarrior         Combat_UseWarriorSkill
//   UseSkillElf             Combat_UseElfSkill
//   SetPlayerStop           FUN_004430c0 — sets idle anim
//   PlayBuffer              FUN_00404bc0 — sound effect by id
//   SetAction               FUN_0043e820 — set entity action
//   sub_4889D0              FUN_004889d0 — skill-attack-finalize helper
//   Net_SendSmallPacket     project helper (Net.h) — does C3 wrap + chain XOR +
//                                                    CSimpleModulus + serial
// ──────────────────────────────────────────────────────────────────────────

// IDA: FUN_0048A180 @ 0x0048A180 — UseSkillElf(c, o).
// This is distinct from SkillElf @ 0048BD70 (the Triple-Shot item helper).
// It emits the common facing packet, snapshots MovementSkillTarget, and has
// only two active families: 26..28 (targeted elf support) and 51 (Ice Arrow).
void __cdecl Combat_UseElfSkill(int c, int o) {
    if (!c || !o || !Hero || !DAT_07cf1ff4)
        return;

    BYTE* const character = (BYTE*)(uintptr_t)c;
    BYTE* const object = (BYTE*)(uintptr_t)o;
    BYTE* const attributes = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    const int targetIndex = (int)DAT_07d780a0;
    if (targetIndex < 0 || targetIndex >= 400)
        return;

    const BYTE gridX = *(BYTE*)(Hero + 904);
    const BYTE gridY = *(BYTE*)(Hero + 908);
    const BYTE facing = (BYTE)(16 * (((int)((*(float*)(Hero + 36) + 22.5f) *
                                           0.022222223f + 1.0f)) & 7));
    BYTE movement[6] = { 0xC1, 6, 0x10, gridX, gridY, facing };
    Net_SendC1Packet(movement, sizeof(movement));

    const BYTE skillId = attributes[(BYTE)DAT_07d7809c + 87];
    BYTE* const target = (BYTE*)(uintptr_t)CharactersClient + 916 * targetIndex;
    *(float*)(character + 788) = *(float*)(target + 16);
    *(float*)(character + 792) = *(float*)(target + 20);
    *(float*)(character + 796) = *(float*)(target + 24);
    *(float*)(object + 36) = FUN_0043e050(
        *(float*)(object + 16), *(float*)(object + 20),
        *(float*)(character + 788), *(float*)(character + 792));

    const WORD targetKey = *(WORD*)(target + 476);
    // 2026-09-02: aca habia un `if (targetKey == 0xFFFF) return;` inventado.
    // UseSkillElf (0x0048A180) no lo tiene: manda el paquete con la key tal cual
    // la lee y despues SIEMPRE anima (SetPlayerMagic o SetPlayerAttack).  Con el
    // guard, un slot en estado raro se comia el skill Y la animacion.
    const DWORD now = GetTickCount();

    if (skillId >= 26 && skillId <= 28) {
        DWORD elapsed = now - DAT_05826cf4;
        if (elapsed > 0x80000000u)
            elapsed = (DWORD)(-(int)elapsed);
        if (elapsed <= 300) {
            FUN_00444a80(c);
            return;
        }
        DAT_05826cf4 = now;
        BYTE packet[6] = { 0xC1, 6, 0x19, skillId,
                           (BYTE)(targetKey >> 8), (BYTE)targetKey };
        Net_SendSmallPacket(packet, sizeof(packet));
        FUN_00444a80(c);
        return;
    }

    if (skillId == 51 && Combat_CheckArrowRequirement()) {
        DWORD elapsed = now - DAT_05826cf4;
        if (elapsed > 0x80000000u)
            elapsed = (DWORD)(-(int)elapsed);
        if (elapsed <= 300) {
            FUN_00444410(c, 0, 0, 0);
            return;
        }
        DAT_05826cf4 = now;
        BYTE packet[6] = { 0xC1, 6, 0x19, 51,
                           (BYTE)(targetKey >> 8), (BYTE)targetKey };
        Net_SendSmallPacket(packet, sizeof(packet));
        FUN_00444410(c, 0, 0, 0);
    }
}

// Declaraciones adelantadas de los helpers que usa Action — casi todos ya están declarados en
// functions.h con los mismos prototipos; los re-declaramos localmente para evitar
// implicit-decl warnings if a particular helper hasn't been wired up yet.
//
// IDA: FUN_004889D0 is ported as Combat_UseWizardSkill in
// stubs_game.cpp y es el emisor de skill directo que usa Action en el case 4.

// Aliases to match IDA companion variable names
//   CharactersClient = g_EntityBase (= DAT_07abf5d0, stride 0x394 / 916)
//   Hero             = DAT_07abf5d8 (local player entity ptr)
#ifndef ACTION_CHARS_CLIENT
// 2026-05-06 BUG-FIX MAYÚSCULO: usar DAT_07abf5d0 directamente, NO el alias
// `g_EntityBase` que está declarado nullptr en stubs.cpp:73 y nunca se
// asigna. ACTION_CHARS_CLIENT + 916*targetIdx + 16 = NULL+0x738 = AV
// (user reportó crash apenas entrar al mundo addr=0x55D1D6 param1=0x738
// 2026-05-06).
#define ACTION_CHARS_CLIENT  ((char*)(uintptr_t)DAT_07abf5d0)
#define ACTION_HERO          ((char*)DAT_07abf5d8)
#endif

// DESVIACION GLOBAL DE ESTA FUNCION (documentada 2026-09-02): los cinco
// `*(BYTE*)(c + 749) = 0` que hay repartidos por los cases NO estan en IDA --
// en el binario Action (0x0048D640) solo LEE ese byte, en L376.  Alli la cola
// la reescribe Player_InputTick en cada click (=1 item, =2 NPC, =3 ataque,
// =4 caminar, =5 skill) y Action corre unicamente cuando MovePath termina
// (0x4ACEF0 L403), asi que no se repite sola.
//
// Nuestro Player_InputTick tiene ademas un "SECONDARY TICK" propio (L910) que
// re-dispara Action cuando `ent[0x2ED] == 3` y el walker esta quieto; sin los
// clears eso se convierte en auto-fire mientras se mantiene el boton.  Los dos
// -- el tick secundario y los clears -- se sacan juntos o no se sacan.
void __cdecl Combat_ProcessQueuedAction(DWORD c, DWORD o)
{
    if (c == 0 || o == 0) return;

    // Decodifica el id de acción encolado: c[+749] es la cabeza de la cola de acciones que setea
    // MoveCharacter cuando el camino termina o se cancela. El valor decodificado es
    // 1-based; 0 means "no action queued".
    int actionId = *(unsigned char*)(c + 749);
    if (actionId == 0) return;
    int caseIdx = actionId - 1;

    float Range = 1.8f;     // default attack range (3 tiles)
    int   targetIdx = DAT_00559ce8;   // hover/attack target (entity slot)

    switch (caseIdx) {

    // ──────────────────────────────────────────────────────────────────────────
    // CASE 0 (action_id=1) — Pickup ground item (IDA: Action 0x48D640 case 0)
    // ──────────────────────────────────────────────────────────────────────────
    case 0: {
        // The native branch uses the latched ItemKey, not SelectedItem.  It
        // has no SetPlayerStop nor queue cleanup side effect.
        const DWORD groundKey = ItemKey;                         // IDA: v114
        const BYTE* const groundItem = &DAT_07e12840[0] + (size_t)groundKey * 0x204;
        const float deltaY = *(float*)(o + 20) - ((float)DAT_07e016c4 * 100.0f + 50.0f);
        const float deltaX = *(float*)(o + 16) - ((float)DAT_07e016c0 * 100.0f + 50.0f);

        if (sqrtf(deltaX * deltaX + deltaY * deltaY) > 150.0f)
            return;

        DAT_00559bec = 6;                                      // IDA: MouseUpdateTimeMax
        const WORD itemType = *(const WORD*)(groundItem + 4);  // IDA: Items[ItemKey][4]
        if (itemType != 463 && !FindEmptySlot_stub(itemType, (int)(uintptr_t)OffsetInventoryItems, 8, 8)) {
            // 0x48F5B7-0x48F61B: report the full inventory and restart the
            // item's short fall animation.  `Items` is a 0x204-byte record;
            // these are the original +0x58/+0x5c world position, +0x60 Z and
            // +0x120 vertical-velocity fields.
            char message[256];
            sprintf(message, GlobalText[375]);                  // IDA: GlobalText + 0x1B774
            UIChatLogWindow_AddText("ERROR", message, 1);      // IDA: byte_7E11E00, MsgType 1
            *(float*)(groundItem + 96) = FUN_004f7500(
                *(const float*)(groundItem + 88), *(const float*)(groundItem + 92)) + 3.0f;
            *(float*)(groundItem + 288) = 50.0f;
            return;
        }
        if (DAT_07e11998 != -1)                                // IDA: SendGetItem
            return;

        DAT_07e11998 = (int)groundKey;
        BYTE pkt[5] = { 0xC1, 0x05, 0x22,
                        (BYTE)(groundKey >> 8), (BYTE)groundKey };
        Net_SendSmallPacket(pkt, sizeof(pkt));
        return;
    }

    // ──────────────────────────────────────────────────────────────────────────
    // CASE 1 (action_id=2) — NPC interact / shop open
    // Original construye packets 0x30 (talk-to-npc) y 0x36 (shop list req).
    // Deferred — UI flow ya está parcialmente cubierto por Game_EnterWorldTick
    // y UI_InGameMenu en otros caminos.
    // ──────────────────────────────────────────────────────────────────────────
    case 1: {
        // Port FIEL de IDA Action (0x0048D640) L813-1189.
        //
        //   MouseUpdateTimeMax = 12;
        //   SetPlayerStop(c);  c[748] = 0;
        //   if (SelectedNpc == -1) return;
        //   v162 = CharactersClient[SelectedNpc] + 747;      // tipo de monstruo
        //   if (v162 < 230) goto LABEL_335;                  // == return
        //   byte_7EAA132 = (v162 == 243 || 246 || 251);
        //   if (g_csQuest[4] == 0xFF) send [C1][03][A0];     // CGQuestInfoRecv
        //   send [C1][05][30][KeyHi][KeyLo];                 // PMSG_NPC_TALK_RECV
        DAT_00559bec = 12;                       // IDA L814: MouseUpdateTimeMax
        FUN_004430c0((int)c);                    // IDA L815: SetPlayerStop
        *(unsigned char*)(c + 748) = 0;          // IDA L816

        const int npcSlotIdx = (int)DAT_00559c70;            // IDA: SelectedNpc
        if (npcSlotIdx < 0 || npcSlotIdx >= 400 || !DAT_07abf5d0)
            return;                                          // IDA L818-820
        char* const npcEnt = (char*)(uintptr_t)DAT_07abf5d0 + npcSlotIdx * 0x394;

        // IDA L822-825: solo se le habla a los tipos >= 230 de Monster.txt.  Es
        // el rango de los NPC (el ultimo monstruo real queda por debajo).
        const BYTE npcType = *(unsigned char*)(npcEnt + 747);  // IDA: v162
        if (npcType < 230)
            return;                                          // IDA: goto LABEL_335

        // IDA L827-830.  El decompile lo escribe con el operador coma
        //   if (v162 == 243 || v162 == 246 || (byte_7EAA132 = 0, v162 == 251))
        //       byte_7EAA132 = 1;
        // que es simplemente: 1 para esos tres tipos, 0 para el resto.
        // 243 = Craftsman, 246 = Weapon Merchant, 251 = Blacksmith (Monster.txt):
        // los tres NPC que reparan.  Este byte es el gate del checkbox de
        // reparacion (lo leen FUN_004e6550 y Chat_InputTick), asi que sin este
        // write el panel del herrero salia sin poder tildar nada.
        DAT_07eaa132 = (npcType == 243 || npcType == 246 || npcType == 251) ? 1 : 0;

        // IDA L832-948: con la quest sin cargar se pide el estado ANTES de
        // hablar.  0xA0 = CGQuestInfoRecv (GameServer/Protocol.cpp:441).
        // El buffer del decompile es `v348[0] = 62980099` = 0x03C10003 -> los
        // bytes 03 00 C1 03 (len WORD + header) y `LOBYTE(v348[1]) = -96`,
        // o sea el paquete de 3 bytes [C1][03][A0].
        if (DAT_00583d8c != 0 &&
            *(unsigned char*)((uintptr_t)DAT_00583d8c + 4) == 0xFF) {
            BYTE questPkt[3] = { 0xC1, 0x03, 0xA0 };
            Net_SendSmallPacket(questPkt, sizeof(questPkt));
        }

        // IDA LABEL_297 (L952-1189): PMSG_NPC_TALK_RECV
        // (GameServer/NpcTalk.h:10) = [C1][05][30][index[2] big-endian].
        // 2026-09-02: se removio un `if (npcKey != 0xFFFF)` inventado -- el
        // binario manda la key tal cual la lee.
        const unsigned short npcKey = *(unsigned short*)(npcEnt + 0x1DC);
        BYTE pkt[5] = { 0xC1, 0x05, 0x30,
                        (BYTE)((npcKey >> 8) & 0xFF), (BYTE)(npcKey & 0xFF) };
        Net_SendSmallPacket(pkt, sizeof(pkt));

        // DESVIACION: IDA no limpia la cola aca (el unico write de c+749 en
        // Action es la LECTURA de L376).  En el binario la cola la reescribe
        // Player_InputTick en cada click y Action solo corre cuando MovePath
        // termina, asi que no se repite.  Nuestro tick secundario si puede
        // volver a entrar, de ahi el clear.
        *(unsigned char*)(c + 749) = 0;
        return;
    }

    // ──────────────────────────────────────────────────────────────────────────
    // CASE 2 (action_id=3) — Attack target (CRITICAL combat path)
    //
    // Flow (per IDA Action.c:1190-1416):
    //  1. Range adjust por weapon type (bow 6.0, polearm 7.0, sword 1.8..2.2).
    //  2. Verifica DAT_00559ce8 (target) != -1.
    //  3. Animation gate por type+anim_state (skip si attack-anim ya activa).
    //  4. Calcula distancia hero→target. Si dentro de Range:
    //     a) SetPlayerAttack(c) — animación local
    //     b) Setea c+757=1 (attack pending), c+784=targetID
    //     c) Setea c+788..796 = posición del target (cache para hit confirm)
    //     d) Manda packet 0x10 (move) — chequea posición del hero
    //     e) Manda packet 0x15 [targetID hi][targetID lo] (attack)
    //  5. Si fuera de range Y move flag (c+444 & 7 == 2):
    //     a) PathFinding2(...) — si ok, c+748=1 (start walk)
    // ──────────────────────────────────────────────────────────────────────────
    case 2: {
        if (targetIdx < 0) {
            *(unsigned char*)(c + 749) = 0;
            return;
        }

        // IDA Action 0x0048D640 L1194-1212 — alcance segun el arma equipada.
        //   v11 = *(__int16 *)(CharacterMachine + 536);   // wear slot 0 (mano izq)
        //   v12 = *(__int16 *)(CharacterMachine + 604);   // wear slot 1 (mano der)
        // OJO: el original lee el global CharacterMachine, NO `c`.  El port leia
        // `c + 536` / `c + 604`, que en la entidad (stride 916) son campos sin
        // relacion, asi que Range nunca salia del default 1.8 y el arco pegaba
        // solo cuerpo a cuerpo.  Los ids son TIPOS de item (sin el +400 del
        // modelo): 136-142 arcos, 128-134 ballestas, 145 el par arco/ballesta.
        const char* const CM = (const char*)(uintptr_t)DAT_07cf1ffc;
        const int leftHandType  = CM ? *(const short*)(CM + 536) : -1;   // IDA: v11
        const int rightHandType = CM ? *(const short*)(CM + 604) : -1;   // IDA: v12

        if (leftHandType >= 96 && leftHandType < 128) Range = 2.2f;       // IDA L1197-1200
        if ((leftHandType >= 136 && leftHandType < 143) ||
            (leftHandType >= 144 && leftHandType < 145) ||
             leftHandType == 146) Range = 6.0f;                           // IDA L1201-1204
        if ((rightHandType >= 128 && rightHandType < 135) ||
             rightHandType == 145) Range = 6.0f;                          // IDA L1205-1208
        if (*(unsigned char*)(c + 747) == 9) Range = 7.0f;                // IDA L1209-1212

        // Saltea si la animación de recuperación está activa
        unsigned char animPrev = *(unsigned char*)(o + 261);
        unsigned short modelType = *(unsigned short*)(o + 2);
        if (modelType == 390) {
            // Hero — skip if anim_state_prev == 81 (skill recovery)
            if (animPrev >= 34u && animPrev <= 91u && animPrev == 81) {
                return;
            }
        } else if (animPrev >= 3u && animPrev <= 4u) {
            return;
        }

        // Compute distance hero — target. Target world pos cached in
        // CharactersClient[targetIdx] @ +16/+20 (floats, cm).
        char* pCharsClient = ACTION_CHARS_CLIENT + 916 * targetIdx;
        float tgtWX = *(float*)(pCharsClient + 16);
        float tgtWY = *(float*)(pCharsClient + 20);

        // Convierte a coordenadas de grilla (cada tile = 100 cm)
        int tgtGX = (int)(tgtWX * 0.01f);
        int tgtGY = (int)(tgtWY * 0.01f);
        // IDA L1231-1232: el original ESCRIBE los globales TargetX/TargetY, no
        // solo locales.  Los lee la rama de fuera-de-alcance de mas abajo
        // (PathFinding2) y tambien el resto del frame (Attack, HeroTile).
        DAT_07e016c0 = (DWORD)tgtGX;                          // IDA: TargetX
        DAT_07e016c4 = (DWORD)tgtGY;                          // IDA: TargetY

        bool forceWalk = (*(unsigned char*)(c + 846) != 0);

        if (!forceWalk) {
            float heroWX = *(float*)(o + 16);
            float heroWY = *(float*)(o + 20);
            float dx = heroWX - ((float)tgtGX * 100.0f + 50.0f);
            float dy = heroWY - ((float)tgtGY * 100.0f + 50.0f);
            float distSq = dx*dx + dy*dy;
            float rangeWorld = Range * 100.0f;

            if (distSq <= rangeWorld * rangeWorld) {
                // ─── In range: send attack ──────────────────────────────────────────────────────────────────────────
                // Local attack animation
                // IDA L1239: MouseUpdateTime = MouseUpdateTimeMax.  Deja el
                // gate de debounce de Player_InputTick pasando en el tick
                // siguiente, que es lo que permite encadenar ataques; sin esto
                // habia que esperar a que el contador volviera a subir.
                DAT_07e11d28 = DAT_00559bec;
                FUN_00444410((int)c, 0, 0, 0);

                *(unsigned char*)(c + 757) = 1;  // attack pending
                // Cache target position for hit confirmation
                *(DWORD*)(c + 788) = *(DWORD*)(pCharsClient + 16);
                *(DWORD*)(c + 792) = *(DWORD*)(pCharsClient + 20);
                *(DWORD*)(c + 796) = *(DWORD*)(pCharsClient + 24);

                // Update facing angle toward target
                *(float*)(o + 36) = FUN_0043e050(
                    *(float*)(o + 16), *(float*)(o + 20),
                    *(float*)(c + 788), *(float*)(c + 792));

                // Mark action consumed and target locked
                *(unsigned char*)(c + 748) = 0;
                *(unsigned char*)(c + 749) = 0;  // clear queue (else loops)
                *(unsigned short*)(c + 784) = (unsigned short)targetIdx;

                // ─── Send packet 0x15 ATTACK ──────────────────────────────────────────────────────────────────────────
                // PMSG_ATTACK_RECV es un paquete C1. El PacketManager del server
                // revierte su chain-XOR antes de despachar; Net_SendBuf después agrega
                // el cifrado de transporte de MuEmu. No se usa C3/CSimpleModulus.
                //
                // Wire format CORRECTO per server PMSG_ATTACK_RECV:
                //   [0xC1][0x07][0x15][TgtH][TgtL][action=0x64][dir]   (plain)
                //   chain-XOR forward bytes 3..6 con LoginKey
                //   MuEmu byte-XOR (HackCheck) toda la trama
                //   raw send (no C3, no CSM)
                //
                // El user reportó: "le pegaba pero a veces de espaldas, a veces
                // se queda atacando al aire" — síntomas de packets que no
                // llegaban válidos al server (CSM-encriptado un C1 no se
                // descifra, server descarta silenciosamente).
                static const BYTE s_AttackKey[32] = {
                    0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                    0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                    0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                    0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
                };
                int targetEntityId = *(short*)(pCharsClient + 476); // entity Id
                // Direction byte: 8-direction code derived from facing angle
                int dirCode = ((int)((*(float*)(o + 36) + 22.5f) *
                                     (1.0f / 45.0f))) & 7;
                BYTE pkt[8];
                pkt[0] = 0xC1;
                pkt[1] = 0x07;
                pkt[2] = 0x15;
                pkt[3] = (BYTE)((targetEntityId >> 8) & 0xFF);
                pkt[4] = (BYTE)(targetEntityId & 0xFF);
                pkt[5] = 0x64;     // AT_ATTACK1 = 100 (0.97k action code)
                pkt[6] = (BYTE)dirCode;
                for (int i = 3; i < 7; ++i) {
                    pkt[i] ^= pkt[i - 1] ^ s_AttackKey[i & 0x1F];
                }
                Net_SendBuf((const char*)pkt, 7);

                PlayBuffer(30, (DWORD)ACTION_HERO, FALSE);
            }
            else if ((*(unsigned char*)(c + 444) & 7) == 2) {
                // Fuera de alcance, con el flag de modo-caminar seteado: pathfind hacia el objetivo
                int heroGX = *(int*)(c + 904);
                int heroGY = *(int*)(c + 908);
                if (Path_FindRoute(heroGX, heroGY, tgtGX, tgtGY,
                                 (unsigned char*)(c + 852), Range)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            }
        }
        return;
    }

    // ──────────────────────────────────────────────────────────────────────────
    // CASE 3 (action_id=4) — Walk to position (final position packet)
    // Per IDA: comprueba que la diff entre heroGX/Y y TargetX/Y sea — 1 tile,
    // entonces manda packet 0x10 con (TargetX, TargetY, heading) y dispara
    // teleport-anim (SetAction 137/138/139/140) en mapas con teleport puzzle
    // (Lorencia case 0/133, Devias case 2/22/55, etc).
    // ──────────────────────────────────────────────────────────────────────────
    case 3: {
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

        // ── 1. Distance gate ──────────────────────────────────────────────────────────────────────────
        int heroGX = *(int*)(c + 904);
        int heroGY = *(int*)(c + 908);
        // 2026-09-04 FIX: el comentario anterior decia "en nuestro build TargetX/Y
        // no son globals" y leia `o + 0x306/0x307`.  Es FALSO: TargetX/TargetY son
        // 0x07E016C0 / 0x07E016C4 (= DAT_07e016c0/c4), los mismos que escriben
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
        int TargetX_v = (int)DAT_07e016c0;
        int TargetY_v = (int)DAT_07e016c4;
        int dxAbs = (heroGX - TargetX_v); if (dxAbs < 0) dxAbs = -dxAbs;
        int dyAbs = (heroGY - TargetY_v); if (dyAbs < 0) dyAbs = -dyAbs;
        int diffAbs = (dxAbs > dyAbs) ? dxAbs : dyAbs;
        if (diffAbs > 1) return;

        // ── 2. World/tile dispatch ──────────────────────────────────────────────────────────────────────────
        int worldIndex = (int)g_GameSubState;
        int TileSub  = (int)DAT_07db8708;
        bool v307 = false;
        bool v309 = false;
        bool overrideFacing = false;

        switch (worldIndex) {
        case 0:  // Lorencia
            switch (TileSub) {
            case 6: case 146:                                  v307 = true; break;
            case 133:                       overrideFacing=true; v309 = true; break;
            case 145:                       overrideFacing=true; v307 = true; break;
            }
            break;
        case 1:  // Dungeon
            switch (TileSub) {
            case 59:                                            v307 = true; break;
            case 60:                        overrideFacing=true; v309 = true; break;
            }
            break;
        case 2:  // Devias
            switch (TileSub) {
            case 22: case 55: case 25:      overrideFacing=true; v307 = true; break;
            case 40:                        overrideFacing=true; v307 = true; break;
            case 45: case 73:                                   v307 = true; break;
            case 91:                        overrideFacing=true; v309 = true; break;
            }
            break;
        case 3:  // Noria
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
            if (TileSub == 8) {                                 v307 = true; }
            else if (TileSub == 38) {
                // Orbe de Noria: flotar (PLAYER_HEALING1 / _FEMALE1).
                *(float*)(o + 36) = *(float*)&_DAT_07e118e4;
                int act = ((*(unsigned char*)(c + 444) & 7) == 2) ? 138 : 137;
                FUN_0043e820((int)o, act);
                float fac = *(float*)(o + 36);
                BYTE heading = (BYTE)((int)((fac + 22.5f) * 0.022222223f + 1.0f) & 7);
                BYTE pkt[8];
                pkt[0] = 0xC1; pkt[1] = 0x06; pkt[2] = 0x18;
                pkt[3] = 0x01; pkt[4] = heading; pkt[5] = 110;
                Combat_SendPlainPacket97k(pkt, 6);
                *(unsigned char*)(c + 854) = 0;
                *(unsigned char*)(c + 749) = 0;
                return;
            }
            break;
        case 7:  // Atlans
            if (TileSub == 39) {            overrideFacing=true; v309 = true; }
            break;
        case 8:  // Tarkan
            // El tile 8 que este case tenia es de Noria (ver el case 3): la cola
            // compartida no es alcanzable con World == 8 salvo por el tile 78, y
            // ahi `tile == 8` es falso.  Queda solo el 78.
            //
            // DESVIACION DOCUMENTADA: para World 8 / tile 78 el decompile termina
            // cayendo igual en LABEL_367 (la cola prueba 8 y 38, y 78 no es
            // ninguno), o sea seria un no-op y el `if (World != 8 || tile != 78)`
            // quedaria sin proposito -- huele a plegado de Hex-Rays.  MU 5.2 tiene
            // `WD_8TARKAN: case 78: Sit`, asi que se mantiene el Sit.
            if (TileSub == 78) {                                v307 = true; }
            break;
        }

        if (overrideFacing) {
            *(float*)((char*)(uintptr_t)DAT_07abf5d8 + 36) = *(float*)&_DAT_07e118e4;
        }

        // ── 4. Send move packet (always, unless cave returned above) ───────────────────────────
        Combat_SendMovePathPacket((int)c, (int)o);
        *(unsigned char*)(c + 854) = 0;

        // ── 5. v309 secondary packet (anim 139/140 + action 109) ───────────────────────────────────────
        if (v309) {
            int act = ((*(unsigned char*)(c + 444) & 7) == 2) ? 140 : 139;
            FUN_0043e820((int)o, act);
            float fac = *(float*)((char*)(uintptr_t)DAT_07abf5d8 + 36);
            BYTE heading = (BYTE)((int)((fac + 22.5f) * 0.022222223f + 1.0f) & 7);
            BYTE pkt[8];
            pkt[0] = 0xC1; pkt[1] = 0x06; pkt[2] = 0x18;
            pkt[3] = 0x01; pkt[4] = heading; pkt[5] = 109;
            Combat_SendPlainPacket97k(pkt, 6);
        }

        // ── 5b. v307 tertiary packet (anim 133/135 + action 108) ───────────────────────────────────────
        if (v307) {
            int act = ((*(unsigned char*)(c + 444) & 7) == 2) ? 135 : 133;
            FUN_0043e820((int)o, act);
            float fac = *(float*)((char*)(uintptr_t)DAT_07abf5d8 + 36);
            BYTE heading = (BYTE)((int)((fac + 22.5f) * 0.022222223f + 1.0f) & 7);
            BYTE pkt[8];
            pkt[0] = 0xC1; pkt[1] = 0x06; pkt[2] = 0x18;
            pkt[3] = 0x01; pkt[4] = heading; pkt[5] = 108;
            Combat_SendPlainPacket97k(pkt, 6);
        }

        // PlayBuffer 30 (sonido de paso) al final, per IDA L2065
        FUN_00404bc0(30, (int)(uintptr_t)DAT_07abf5d8, 0);
        *(unsigned char*)(c + 749) = 0;
        return;
    }

    // ──────────────────────────────────────────────────────────────────────────
    // CASE 4 (action_id=5) — Cast skill
    //
    // Flow (per IDA Action.c:2244-2583):
    //  1. Resolve skill type from DAT_07d78098/DAT_07d7809c (+ skill table).
    //  2. Validate target (DAT_07d780a0 = MovementSkillTarget) is alive.
    //  3. Skill type dispatch:
    //     a) Direct-target skills (1-4,7,11,17): in-range — sub_4889D0(c,o);
    //        out-of-range — PathFinding2 — c+748=1
    //     b) Magic skills (19-23,43,49,56): in-range — UseSkillElf,
    //        out-of-range — PathFinding2
    //     c) AOE skills (26,27,28): same as magic but require LOS
    //     d) Warrior melee skills (47): in-range — UseSkillWarrior(c,o);
    //        fuera de alcance — PathFinding2 con alcance ×1.2
    //     e) Special heal/buff (51): same as elf magic but LOS-gated
    // ──────────────────────────────────────────────────────────────────────────
    case 4: {
        // Action 0x48D640 does not always read Hero+913.  Its queued path
        // preserves the direct selector when dword_7D78098 is clear.
        const BYTE* const attributes = (const BYTE*)(uintptr_t)DAT_07cf1ff4;
        const int skillType = DAT_07d78098
            ? (attributes ? attributes[(BYTE)DAT_07d7809c + 87] : 0)
            : (int)DAT_07d7809c;                             // IDA: v62

        int skillTarget = (int)DAT_07d780a0;  // MovementSkillTarget
        if (skillTarget < 0) {
            return;
        }
        char* pTgt = ACTION_CHARS_CLIENT + 916 * skillTarget;
        if (*(unsigned char*)(pTgt + 765) != 0) {
            return;
        }

        int tgtGX = (int)(*(float*)(pTgt + 16) * 0.01f);
        int tgtGY = (int)(*(float*)(pTgt + 20) * 0.01f);
        // IDA L2302/2305, L2380/2381, L2427/2428, L2495/2496: los cuatro
        // sub-casos escriben los globales TargetX/TargetY antes del chequeo
        // de alcance.
        DAT_07e016c0 = (DWORD)tgtGX;                      // IDA: TargetX
        DAT_07e016c4 = (DWORD)tgtGY;                      // IDA: TargetY

        int skillRange = Combat_GetSkillRange97k(skillType);

        float dx = *(float*)(o + 16) - ((float)tgtGX * 100.0f + 50.0f);
        float dy = *(float*)(o + 20) - ((float)tgtGY * 100.0f + 50.0f);
        float dist = dx*dx + dy*dy;
        float rng  = (float)skillRange * 100.0f;
        bool inRange = (dist <= rng * rng);
        bool forceWalk = (*(unsigned char*)(c + 846) != 0);

        // Lee la posición de grilla del héroe (encriptada, pero en nuestro build es un DWORD plano)
        int heroGX = *(int*)(c + 904);
        int heroGY = *(int*)(c + 908);

        switch (skillType) {

        // ─── Direct-target skills (Falling Slash, Lunge, Uppercut, Cyclone,
        //                          Slash, Triple Shot, Sleep) ??????
        case 1: case 2: case 3: case 4: case 7: case 11: case 17:
            if (inRange && !forceWalk) {
                if (Path_IsLineClear(heroGX, heroGY, tgtGX, tgtGY)) {
                    // Direct-target wizard path (UseSkillWizard @ 004889D0).
                    Combat_UseWizardSkill((DWORD)c, (DWORD)o);
                }
            } else {
                if (Path_FindRoute(heroGX, heroGY, tgtGX, tgtGY,
                                 (unsigned char*)(c + 852), (float)skillRange)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            }
            break;

        // Action.c LABEL_123: estos IDs comparten el camino de duración de guerrero con
        // el gate de alcance ×1.2. No usan el helper de mago acá.
        case 19: case 20: case 21: case 22: case 23:
        case 43: case 49: case 56:
        case 47:
            if (skillType == 47 && *(unsigned short*)(ACTION_HERO + 696) != 818) {
                return;
            }
            if ((float)skillRange * 1.2f * 100.0f < (float)sqrt((double)dist) || forceWalk) {
                if (Path_FindRoute(heroGX, heroGY, tgtGX, tgtGY,
                                 (unsigned char*)(c + 852),
                                 (float)skillRange * 1.2f)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            } else {
                Combat_UseWarriorSkill((int)c, (int)o);  // UseSkillWarrior
            }
            break;

        // Targeted Elf support skills (Heal, Greater Defense, Greater Damage).
        // El dispatch usa UseSkillElf, que manda C3:19 con este objetivo.
        case 26: case 27: case 28:
            if (inRange && !forceWalk) {
                if (Path_IsLineClear(heroGX, heroGY, tgtGX, tgtGY)) {
                    Combat_UseElfSkill((int)c, (int)o);  // UseSkillElf (AOE)
                }
                // 2026-09-02: aca habia un `else if (Path_FindRoute(...))` que
                // hacia caminar al heroe cuando CheckWall fallaba.  IDA
                // (LABEL_184) no hace nada en ese caso: si la linea de vista
                // esta cortada, el skill simplemente no sale y la accion queda
                // encolada hasta el proximo click.
            } else {
                if (Path_FindRoute(heroGX, heroGY, tgtGX, tgtGY,
                                 (unsigned char*)(c + 852), (float)skillRange)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            }
            break;

        // El skill 51 sigue el mismo dispatch de alcance / línea de vista en IDA.
        case 51:
            if (inRange && !forceWalk) {
                if (Path_IsLineClear(heroGX, heroGY, tgtGX, tgtGY)) {
                    Combat_UseElfSkill((int)c, (int)o);
                }
                // 2026-09-02: aca habia un `else if (Path_FindRoute(...))` que
                // hacia caminar al heroe cuando CheckWall fallaba.  IDA
                // (LABEL_184) no hace nada en ese caso: si la linea de vista
                // esta cortada, el skill simplemente no sale y la accion queda
                // encolada hasta el proximo click.
            } else {
                if (Path_FindRoute(heroGX, heroGY, tgtGX, tgtGY,
                                 (unsigned char*)(c + 852), (float)skillRange)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            }
            break;

        default:
            return;
        }

        // Nota: después del dispatch, IDA hace el refcount anti-tamper de hash al final
        // SkillAttribute entry — skipped per project policy.
        // The skill helpers (UseSkillWarrior/Elf) handle their own packet
        // 0x19 send (PMSG_SKILL_ATTACK [skill_lo][skill_hi][tgtH][tgtL]).
        return;
    }

    default:
        return;
    }
}

