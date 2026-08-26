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
// ─── ENTITY_MELEEATTACKSTART (0x00474310) ──────────────────────────────────────────────────────────────────────────
//
//   void Entity_MeleeAttackStart(int entity_ptr):
//     FUN_00443e70()                      — reset sonido/efecto global
//     Entity_SetAnimation(entity, 0x57)   — anim de ataque melee
//     entity[+0x108] = 5.0f              — frame time de ataque
//     entity[+0x7C]  = 3                 — attack_state = atacando
//     entity[+0x164] = 1.0f             — attack_intensity
//     FUN_00460dc0(0x498,               // efecto 0x498 = slash/hit particle
//                  world_x, world_z, entity[+0xE8], ...)
//     FUN_00404bc0(0x58, 0, 0)           — UI event: mostrar daño recibido
//
//   FUN_00460dc0 @ aprox 0x00460dc0 = Particle_Spawn(effect_id, x, z, height, ...)
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
//       - FUN_00460dc0(effect_id, ...) — spawn efecto visual del skill
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
//   Entity_MeleeAttackStart @ 0x00474310 — melee hit + particle + UI
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
static void Combat_SendDurationSkill97k(char* entity, int skillType, int action,
                                        BYTE direction = 0, BYTE destination = 0,
                                        BYTE angle = 0, WORD targetKey = 0xFFFF,
                                        int packetX = -1, int packetY = -1);
static void Combat_SendAreaHits97k(char* entity, int skillType);
static void Combat_SendPlainPacket97k(BYTE* pkt, int len);

// Attack.c LABEL_1354: después de que CheckTarget llenó c+788/c+792 desde el
// personaje bajo el mouse o desde el pick de terreno, el original mira hacia ese punto antes
// dispatching the 5/8/9/12 area-skill switch.
static void Combat_AimAtCheckedTarget97k(char* entity, bool hasTarget)
{
    if (!entity || !hasTarget) return;
    *(float*)(entity + 36) = FUN_0043e050(
        *(float*)(entity + 16), *(float*)(entity + 20),
        *(float*)(entity + 788), *(float*)(entity + 792));
}

// Base de la grilla de inventario — la usa el feedback sonoro del mana-scroll de Attack.
// Defined in src/Render/HUD_Pass3.cpp.
extern "C" BYTE OffsetInventoryItems[];

// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 6475-7690 (1216 lines)
// FUN_00491c40 (Send_MovePacket), FUN_0049cbf0 (Attack), FUN_0048ba70 (CheckArrow),
// FUN_0048a180 (UseSkillElf stub), FUN_0048d640 (Action big switch),
// + Send_MovePacket_Player_legacy_stub, FUN_004f6c30 (Terrain_GetAttrDirect)
// =============================================================================
// FUN_00491c40 @ 0x00491C40 — Send_MovePacket(entity_ptr, player_entity_ptr)
// Sends opcode 0x10 movement packet: C1 len 10 wp_count target_x target_y facing path[wp_count]
// Codifica con XOR usando la clave hardcodeada de 32 bytes. Saltea si la entidad tiene el bit 0x20 en +0x78.
// wp_count se limita a 0xe. Setea DAT_00559bec = pkt_size_code.
void __cdecl FUN_00491c40(int param_1, int param_2)
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
    // fijo. Per IDA decompile FUN_00491c40 + server CGMoveRecv:
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
    // 2026-05-05 BUG-FIX: IDA decompile FUN_00491c40 muestra que pkt[3]/pkt[4]
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
// FUN_0049cbf0 @ 0x0049CBF0 — Attack(c)  [PORTED 2026-05-05]
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
//   FUN_0043f3e0 = PathFinding2(sx, sy, tx, ty, path_buf, radius) — uint
//   FUN_00483160 = CheckAttack() — uint (0 if locked)
//   FUN_0049cae0 = CheckTarget_stub(c) — bool
//
// HeroKey se lee del campo +0x1DC de la entidad del héroe (= g_HeroKey que asigna
// Net_Process at JoinServer ACK; see Net_Process.cpp:1294).

// El `g_dwLatestMagicTick` de IDA limita los envíos de paquetes de skill a uno cada 300ms.
// Nosotros usamos un static local a la función (no la dirección de IDA, que en nuestro
// maps to NPC dialog state).
static DWORD g_dwLatestMagicTick_Attack = 0;

// Attack.c mantiene el Teleport con su propio gate de pedido de 3000 ms. La dirección
// original aliasa un timer de UI/NPC en este cliente reconstruido, así que conservamos
// la misma transición de estado sin reusar ese timer, que no tiene nada que ver.
static DWORD g_dwLatestTeleportRequest_Attack = 0;

extern "C" int g_MouseOnWindow;   // Game/Player_InputTick.cpp

void __cdecl FUN_0049cbf0(void *entity_v)
{
    char* entity = (char*)entity_v;
    if (!entity) return;

    // Diagnóstico de una sola vez para una pulsación física del botón derecho. Va a propósito
    // antes de cada gate, para que un log en runtime marque la primera condición que falla
    // sin producir una línea por frame mientras el botón está apretado.
    const bool rightButtonPush = (DAT_083a42d0 != 0);
    static bool s_rmbWasDown = false;
    const bool rightButtonHeld = (DAT_083a42ac != 0);
    if (rightButtonHeld && !s_rmbWasDown) {
        char trace[192];
        wsprintfA(trace,
            "SKILL RMB Attack-entry push=%u held=%u inputLock=%u edit=%u hero=%08X",
            (unsigned)DAT_083a42d0, (unsigned)DAT_083a42ac,
            (unsigned)DAT_07e11dc0, (unsigned)DAT_07e11d30,
            (unsigned)(uintptr_t)Hero);
        DbgLogPublic(trace);
    }
    s_rmbWasDown = rightButtonHeld;
    if (rightButtonPush) {
        char trace[256];
        wsprintfA(trace,
            "SKILL RMB enter hero=%08X attr=%08X slot=%u selected=%d state=%u action=%u flags=%u dead=%u",
            (unsigned)(uintptr_t)Hero, (unsigned)(uintptr_t)DAT_07cf1ff4,
            Hero ? (unsigned)*(BYTE*)(Hero + 913) : 0u, (int)SelectedCharacter,
            (unsigned)DAT_07e11dc0, (unsigned)*(BYTE*)(entity + 261),
            (unsigned)(*(BYTE*)(entity + 444) & 7), (unsigned)*(BYTE*)(entity + 765));
        DbgLogPublic(trace);
    }

    // 1) Early exits — anti-tamper / loading state ──────────────────────────────────────────────────────────────────────────
    if (DAT_07e11d30 != 0) return;          // EditFlag: editor mode active

    // 2) Stamina decay on entity (movement-tick interpolation) ────────────────────────────────────────────────
    if (entity[0x7c] == 1) {
        float& fAnim = *(float*)(entity + 0x168);
        fAnim -= 0.1f;                       // IDA: -= 0.1
        if (fAnim < 0.1f) entity[0x7c] = 2;
    }

    // 3) Mouse button gate ──────────────────────────────────────────────────────────────────────────
    bool bSuccess = false;
    if (DAT_07e11dc0 != 0) {
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
            && DAT_07e11984 == 2                              // Attacking
            && SelectedCharacter != -1;                            // SelectedCharacter
        if (!autoAttackOK && !bSuccess) {
            // IDA resetea acá RButtonPressTime=0 y RButtonPopTime=WorldTime.
            // No tenemos esos globals mapeados (los DAT_07e11dc4/dc8 de nuestro
            // build sirven para otra cosa: estado del diálogo de NPC, NO tocar).
            // Saltear el reset del timer es seguro porque ese timer alimenta sólo
            // el chequeo de mantener apretado, que también salteamos.
            return;
        }
        // Cap hold time to 3600 seconds (anti-bot)
        // Skipped: timer comparison — gameplay non-critical.
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
    int iType = 0;
    if (Hero != 0) {
        iType = (int)Combat_ResolveQueuedSkillId97k();
    }
    if (rightButtonPush) {
        char trace[256];
        const BYTE* attr = (const BYTE*)(uintptr_t)DAT_07cf1ff4;
        wsprintfA(trace,
            "SKILL RMB resolved skill=%d attrSkill=%u mana=%u ag=%u target=(%u,%u)",
            iType, (attr && Hero) ? (unsigned)attr[*(BYTE*)(Hero + 913) + 87] : 0u,
            attr ? (unsigned)*(const WORD*)(attr + 30) : 0u,
            attr ? (unsigned)*(const WORD*)(attr + 36) : 0u,
            (unsigned)DAT_07e016c0, (unsigned)DAT_07e016c4);
        DbgLogPublic(trace);
    }

    // 6) Segundo gate de SafeZone (IDA Attack L1314): dentro del pueblo solo
    //    pasan los skills 26/27/28 (buffs de Elf), 16 y 48 (Greater Life).
    if (entity[846] != 0 && iType != 26 && iType != 27 && iType != 28
        && iType != 16 && iType != 48) {
        return;
    }

    // 7) 0049CBF0 escribe Attacking desde el estado de movimiento de la entidad. Ése es
    // el estado que consume el gate de auto-ataque de la invocación siguiente, arriba.
    if (DAT_07e11e18 != 0 && (int)g_GameSubState != 6) {
        const BYTE movementState = entity[444] & 7;
        if ((movementState != 2 || iType == 24 || iType == 25 || iType == 52)
            && (movementState != 1 || iType != 48)) {
            DAT_07e11984 = 2;
        } else {
            DAT_07e11984 = -1;
        }
    }

    // 8) MouseOnWindow (IDA Attack L1330) — si el cursor esta sobre una ventana
    // de UI, no se ataca.  2026-08-15: estaba diferido ("no trackeamos ese
    // global"), pero SI existe: `g_MouseOnWindow`, que puebla
    // `MouseOnWindow_Update` en Player_InputTick.cpp (y al que el widget de chat
    // le pasa su latch `g_ChatLB_MouseOnWindow`).  Sin este gate, click derecho
    // sobre el inventario / chat / paneles disparaba el skill igual.
    if (g_MouseOnWindow) return;

    // 8b) Gate por estado de animacion (IDA Attack L1335-1345).  `c+261` es el
    // action-state; el heroe (entity_type 390) solo puede atacar desde los
    // estados 1..12 y el 78, y cualquier otra entidad solo desde 0 o 1.  Sin
    // esto se podia relanzar el skill durante la animacion de recuperacion.
    {
        const BYTE actionState = (BYTE)entity[261];
        if (*(short*)(entity + 2) == 390) {
            if (actionState == 0 || (actionState > 0x0C && actionState != 78))
                return;
        } else if (actionState > 1) {
            return;
        }
    }

    // 9) iType 0 (no skill equipped) — fall through to basic attack path ──────────────────
    // Basic attack (no skill) is sent from Action() @ 0x48D640 / Player_InputTick
    // ya (opcode 0x15 desde Player_InputTick.cpp:594). Acá no hay nada que hacer.
    if (iType == 0) {
        return;
    }

    // 10) Anti-cheat string check (aWebzen_*) — pass-through in clean builds.
    //     Salteado: si no, escanearía Hero+449 buscando el marcador "WEBZEN".

    // 11) Chequeo de recursos contra CharacterAttribute. IDA lee los dos WORD en
    // decimal offsets +30 (current mana) and +36 (current AG).
    // GetSkillInformation(iType, 1, 0, &piMana, 0, &piSkillMana)
    int piManaLocal = 0, piDistLocal = 0, piSkillManaLocal = 0;
    GetSkillInformation(iType, 1, 0, &piManaLocal, &piDistLocal, &piSkillManaLocal);
    BYTE* charAttrBase = (BYTE*)DAT_07cf1ff4;
    if (!charAttrBase) return;
    unsigned short curMana = *(unsigned short*)(charAttrBase + 30);
    unsigned short curEner = *(unsigned short*)(charAttrBase + 36);

    if (piManaLocal > curMana) {
        // Intenta usar un mana scroll del inventario (tipo de slot 3 = categoría scroll)
        // Returns slot index or -1.
        int scrollSlot = (int)FUN_00482be0(3);
        if (scrollSlot == -1) return;     // no mana scroll
        // Block on warehouse/trade open
        if (WarehouseOpened || TradeOpened) {
            // GlobalText[474] = "No puedes hacer esto ahora."
            UIChatLogWindow_AddText("ERROR", GlobalText[474], 2);
            return;
        }
        if (EnableUse > 0) return;
        EnableUse = 10;
        // Send packet 0x26 (PMSG_USE_INVITEM)
        // [0xC1][0x05][0x26][slot+12:1][0:1]
        unsigned char pkt[6];
        pkt[0] = 0xC1; pkt[1] = 5; pkt[2] = 0x26;
        pkt[3] = (unsigned char)(scrollSlot + 12);
        pkt[4] = 0;
        Net_SendSmallPacket(pkt, 5);
        // 2026-05-08: sound feedback per IDA Attack.c. OffsetInventoryItems
        // es una grilla de stride 0x44 donde slot[0] es el campo int Type. Mapeado
        // for our build: type 448 = mana potion (sound 33), 449-457 = HP /
        // misc potions / scrolls (sound 32).
        {
            int itemType = *(int*)(OffsetInventoryItems + scrollSlot * 0x44);
            if (itemType == 448) {
                FUN_00404bc0(33, 0, 0);
            } else if (itemType >= 449 && itemType <= 457) {
                FUN_00404bc0(32, 0, 0);
            }
        }
        return;
    }
    if (piSkillManaLocal > curEner) {
        return;  // not enough Energy/SP
    }

    // 12) CheckTarget — mira hacia el objetivo, setea entity[+908]/[+904] (gridY/gridX) ???
    bool bHasTarget = (CheckTarget_stub((DWORD)entity) != 0);
    // Esto se limita a propósito a los picks de piso fallidos mientras el botón derecho está apretado.
    // Las ramas del original conservan el facing anterior cuando CheckTarget falla;
    // loguear este límite distingue un fallo del picker/frustum de un error en la matemática del ángulo.
    if (rightButtonPush && !bHasTarget) {
        static DWORD s_lastTargetMissTrace = 0;
        const DWORD now = GetTickCount();
        if (now - s_lastTargetMissTrace >= 250) {
            s_lastTargetMissTrace = now;
            char targetMissTrace[176];
            _snprintf_s(targetMissTrace, _countof(targetMissTrace), _TRUNCATE,
                        "SKILL PICK MISS skill=%d selected=%d pickGrid=(%.1f,%.1f) target=(%.1f,%.1f)",
                        iType, (int)SelectedCharacter,
                        *(float*)&DAT_080ab288, *(float*)&DAT_080ab28c,
                        *(float*)(entity + 788), *(float*)(entity + 792));
            DbgLogPublic(targetMissTrace);
        }
    }
    if (rightButtonPush && iType != 0 && iType != 9 && iType != 10 &&
        iType != 12 && iType != 14) {
        char targetTrace[192];
        int selected = (int)SelectedCharacter;
        unsigned type = 0;
        unsigned key = 0xFFFF;
        if (selected >= 0 && selected < 400 && CharactersClient) {
            const char* chosen = (const char*)CharactersClient + selected * 916;
            type = (unsigned)(BYTE)chosen[132];
            key = (unsigned)*(const WORD*)(chosen + 476);
        }
        wsprintfA(targetTrace,
                  "SKILL TARGET skill=%d selected=%d check=%d type=%u key=%u range=%d",
                  iType, selected, bHasTarget ? 1 : 0, type, key,
                  Combat_GetSkillRange97k(iType));
        DbgLogPublic(targetTrace);
    }
    if (SelectedCharacter != -1) {
        Combat_SeedRuntimeState97k(iType, (int)SelectedCharacter);
    } else {
        DAT_05826d10 = (DWORD)iType;
    }

    // 13) Per-skill-type dispatch ──────────────────────────────────────────────────────────────────────────
    // IDA 0049CBF0 maneja Hell Fire e Inferno como cases directos del switch,
    // before generic target-follow range processing.  Neither requires a
    // selected character for a ground cast.
    if (iType == 10 || iType == 14) {
        if (*(unsigned short*)(entity + 2) != 390)
            return;
        Combat_AimAtCheckedTarget97k(entity, bHasTarget);
        const BYTE direction = (BYTE)(int)(*(float*)(entity + 36) * (256.0f / 360.0f));
        Combat_SendDurationSkill97k(entity, iType, (iType == 10) ? 90 : 89,
                                    direction, 0, 0, 0xFFFF);
        Combat_SendAreaHits97k(entity, iType);
        return;
    }
    // Evil Spirit and Aqua Beam are no-target area casts.  A selected monster
    // no tiene que transformar ninguno de los dos en el camino de sólo-C3:1E con objetivo: eso omite
    // C3:1D and leaves the server with no multi-hit list.
    if (iType == 9 || iType == 12) {
        Combat_AimAtCheckedTarget97k(entity, bHasTarget);
        const BYTE direction = (BYTE)(int)(*(float*)(entity + 36) * (256.0f / 360.0f));
        Combat_SendDurationSkill97k(entity, iType, (iType == 12) ? 88 : -1,
                                    direction, 0, 0, 0xFFFF);
        Combat_SendAreaHits97k(entity, iType);
        return;
    }

    // Attack.c case 6 (Teleport): esto no es un paquete de ataque mágico. MuEmu
    // recibe su PMSG_TELEPORT_RECV nativo como C1:06:1C:gate:x:y, donde un
    // gate en cero indica el Teleport de posición libre del mago. La posición
    // recién se confirma después, con la respuesta C3:1C que maneja Net_Process.cpp.
    if (iType == 6) {
        if (!bHasTarget || DAT_07e91388 != 0 || DAT_05826d04 != 0)
            return;

        const BYTE currentAction = (BYTE)entity[261];
        if ((*(WORD*)(entity + 2) == 390 && currentAction == 87) ||
            (*(WORD*)(entity + 2) != 390 && currentAction == 5))
            return;
        if ((BYTE)entity[124] == 1 || (BYTE)entity[124] == 2 ||
            *(float*)(entity + 360) < 0.7f)
            return;

        const BYTE targetX = (BYTE)DAT_07e016c0;
        const BYTE targetY = (BYTE)DAT_07e016c4;
        extern unsigned char* TerrainWall;
        BYTE terrainAttr = TerrainWall[((unsigned)targetY << 8) | targetX];
        if ((terrainAttr & 0x20) != 0)
            terrainAttr &= (BYTE)~0x20;
        if (terrainAttr != 0)
            return;

        const DWORD now = GetTickCount();
        if (now - g_dwLatestTeleportRequest_Attack < 3000)
            return;

        *(float*)(entity + 36) = FUN_0043e050(
            *(float*)(entity + 16), *(float*)(entity + 20),
            *(float*)(entity + 788), *(float*)(entity + 792));
        DAT_05826d04 = 1;
        g_dwLatestTeleportRequest_Attack = now;

        BYTE packet[6] = { 0xC1, 0x06, 0x1C, 0x00, targetX, targetY };
        Net_SendSmallPacket(packet, sizeof(packet));
        return;
    }

    if (iType < 30 && iType != 15 && iType != 16) {
        // LABEL_240 path: skills 1..29 (DK basic skills, low-level magic).
        // Setea MovementSkillTarget = SelectedCharacter para que el pathfind lo siga.
        auto seedRuntimeTarget = [&](int skillId) {
            if (SelectedCharacter == -1) return;
            DAT_07d78098 = 1;
            DAT_07d7809c = (Hero ? *(unsigned char*)(Hero + 913) : 0);
            DAT_07d780a0 = (DWORD)SelectedCharacter;
            DAT_05826d10 = (DWORD)skillId;
        };
        if (SelectedCharacter != -1) {
            DAT_07d78098 = 1;
            DAT_07d7809c = (Hero ? *(unsigned char*)(Hero + 913) : 0);
            DAT_07d780a0 = (DWORD)SelectedCharacter;
        }

        // SkillAttribute is a struct array stride 40. Field +0x26 (=38 bytes)
        // es "Distance" (alcance del skill en tiles). El índice de IDA es (40*iType+38),
        // y SkillAttribute se puede indexar por byte.
        int skillRange = Combat_GetSkillRange97k(iType);

        // Distance check: hero world coords vs target tile center.
        float tx = (float)(int)DAT_07e016c0 * 100.0f + 50.0f;
        float ty = (float)(int)DAT_07e016c4 * 100.0f + 50.0f;
        float dx = *(float*)(entity + 16) - tx;
        float dy = *(float*)(entity + 20) - ty;
        float dist2 = dx * dx + dy * dy;
        float maxDist = (float)skillRange * 100.0f;
        if (rightButtonPush && SelectedCharacter != -1) {
            char rangeTrace[160];
            _snprintf_s(rangeTrace, _countof(rangeTrace), _TRUNCATE,
                        "SKILL RANGE skill=%d dist2=%.0f limit2=%.0f hero=(%.0f,%.0f) point=(%.0f,%.0f)",
                        iType, dist2, maxDist * maxDist,
                        *(float*)(entity + 16), *(float*)(entity + 20), tx, ty);
            DbgLogPublic(rangeTrace);
        }
        if (dist2 > maxDist * maxDist) {
            // Fuera de alcance — pathfind hacia el objetivo.
            if (SelectedCharacter != -1
                && *(unsigned char*)((unsigned char*)(CharactersClient + 916 * (int)SelectedCharacter) + 132) == 1) {
                int gx = *(int*)(entity + 904);
                int gy = *(int*)(entity + 908);
                if (FUN_0043f3e0(gx, gy, (int)DAT_07e016c0, (int)DAT_07e016c4,
                                 (unsigned char*)(entity + 852), (float)skillRange) != 0) {
                    entity[748] = 1;  // queue movement action
                    entity[749] = 5;  // action type = follow
                }
            }
            return;
        }
        // In range — execute attack.
        if (SelectedCharacter != -1) {
            unsigned char ttype = *(unsigned char*)((unsigned char*)(CharactersClient + 916 * (int)SelectedCharacter) + 132);
            if (ttype == 1 && iType >= 26 && iType <= 28) {
                // Elf buff (Heal/Greater Defense/Greater Damage) — UseSkillElf
                seedRuntimeTarget(iType);
                FUN_0048a180((int)entity, (int)entity);
                return;
            }
        }
        if (iType == 0x18) {
            seedRuntimeTarget(iType);
            FUN_0048a180((int)entity, (int)entity);
            return;
        }
        // Attack.c's later low-skill phase handles these through C3:1E.  It
        // primero requiere CheckTarget, rota hacia la posición de impacto cacheada,
        // y después elige una animación local distinta para el Aqua Beam.
        if (iType == 5 || iType == 8 || iType == 9 || iType == 12) {
            if (!bHasTarget) {
                return;
            }
            *(float*)(entity + 36) = FUN_0043e050(
                *(float*)(entity + 16), *(float*)(entity + 20),
                *(float*)(entity + 788), *(float*)(entity + 792));

            WORD targetKey = 0xFFFF;
            if (SelectedCharacter != -1 && FUN_00483160() != 0) {
                targetKey = *(unsigned short*)(CharactersClient + 916 * (int)SelectedCharacter + 476);
            }
            const BYTE direction = (BYTE)((int)(*(float*)(entity + 36) * (256.0f / 360.0f)));
            if (iType == 5) {
                // Flame usa la posición de mundo cacheada del objetivo, a diferencia de
                // los otros tres, que usan las coordenadas de grilla del héroe.
                const int packetX = (int)(*(float*)(entity + 788) * 0.01f);
                const int packetY = (int)(*(float*)(entity + 792) * 0.01f);
                Combat_SendDurationSkill97k(entity, iType, -1, direction, 0, 0,
                                            targetKey, packetX, packetY);
            } else {
                Combat_SendDurationSkill97k(entity, iType, (iType == 12) ? 88 : -1,
                                            direction, 0, 0, targetKey);
            }
            return;
        }
        int heroGX = *(int*)(entity + 904);
        int heroGY = *(int*)(entity + 908);
        if (iType >= 26 && iType <= 28) {
            if (FUN_004830b0(heroGX, heroGY, (int)DAT_07e016c0, (int)DAT_07e016c4)) {
                seedRuntimeTarget(iType);
                FUN_0048a180((int)entity, (int)entity);
            } else if (FUN_0043f3e0(heroGX, heroGY, (int)DAT_07e016c0, (int)DAT_07e016c4,
                                    (unsigned char*)(entity + 852), (float)skillRange) != 0) {
                entity[748] = 1;
                entity[749] = 5;
            }
            return;
        }
        if (iType == 1 || iType == 2 || iType == 3 || iType == 4 ||
            iType == 7 || iType == 0x0b || iType == 0x0d || iType == 0x10 || iType == 0x11 ||
            iType == 43 || iType == 49) {
            seedRuntimeTarget(iType);
            UseSkillWizard_stub((DWORD)entity, (DWORD)entity);
            return;
        }
        // Hell Fire (10) e Inferno (14) siguen por C3:1E con el
        // hero grid, 256-step facing, and a CheckAttack-gated target key.
        if ((iType == 10 || iType == 14) && *(unsigned short*)(entity + 2) == 390) {
            WORD targetKey = 0xFFFF;
            if (SelectedCharacter != -1 && FUN_00483160() != 0) {
                targetKey = *(unsigned short*)(CharactersClient + 916 * (int)SelectedCharacter + 476);
            }
            const BYTE direction = (BYTE)(int)(*(float*)(entity + 36) * (256.0f / 360.0f));
            Combat_SendDurationSkill97k(entity, iType, (iType == 10) ? 90 : 89,
                                        direction, 0, 0, targetKey);
            return;
        }
        // 97k real no deja el resto de los skills bajos caer por un 0x19 genérico.
        // Si no matcheó alguno de los helpers/runtime paths explícitos, se corta acá.
        return;
    }

    // 14) iType == 52 (Twister/Cyclone) — packet 0x1E ──────────────────────────────────────────────────────────────────────────
    if (iType == 52 && SelectedCharacter != -1 && bHasTarget) {
        int tgtGX = (int)DAT_07e016c0;
        int tgtGY = (int)DAT_07e016c4;
        int heroGX = *(int*)(entity + 904);
        int heroGY = *(int*)(entity + 908);
        int skillRange = Combat_GetSkillRange97k(iType);
        float tx = (float)tgtGX * 100.0f + 50.0f;
        float ty = (float)tgtGY * 100.0f + 50.0f;
        float dx = *(float*)(entity + 16) - tx;
        float dy = *(float*)(entity + 20) - ty;
        float dist2 = dx * dx + dy * dy;
        float maxDist = (float)skillRange * 100.0f;
        if (dist2 <= maxDist * maxDist) {
            // El case 52 en rango de 0049CBF0 escribe c+36 desde la posición de mundo
            // del objetivo chequeado, justo antes de emitir C3:1E.
            Combat_AimAtCheckedTarget97k(entity, true);
            Combat_SeedRuntimeState97k(iType, (int)SelectedCharacter);
            UseSkillWizard_stub((DWORD)entity, (DWORD)entity);
        } else if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                (unsigned char*)(entity + 852), (float)skillRange) != 0) {
            entity[748] = 1;
            entity[749] = 5;
        }
        return;
    }

    // 2026-08-24 FIX (Soul Barrier no se podia castear sobre uno mismo):
    // aca habia un early-return inventado —
    //     if (iType == 16 && SelectedCharacter == -1) return;
    // con el comentario "Attack.c case 16 has no self-cast path". Es falso.
    // IDA (0x49CBF0 L7134-7140) hace lo contrario: SIN target no corta, sino
    //     if ( SelectedCharacter != -1 ) MovementSkillTarget = SelectedCharacter;
    //     else                           MovementSkillTarget = 0;
    // y sigue al chequeo de rango y al envio. El gate de party de abajo SI es
    // fiel, pero esta dentro de `if (SelectedCharacter != -1)`, o sea solo
    // aplica cuando apuntas a alguien. Sin target el skill se castea sobre
    // uno mismo — confirmado contra el cliente original, y no es algo que
    // agregue el DLL.
    if (iType == 16 && SelectedCharacter == -1) {
        // El target del self-cast es el propio heroe.
        //
        // DESVIACION documentada: IDA pone `MovementSkillTarget = 0` (L7138), pero
        // nuestro `UseSkillWizard_stub` usa ese indice como el target REAL —
        // `CharactersClient[MovementSkillTarget]` — y corta con
        // `if (targetIdx < 0 || targetIdx >= 400) return;`. Con 0 apuntaria a una
        // entidad cualquiera del slot 0; con -1 (lo que se probo primero) el
        // UseSkillWizard salia sin hacer nada — medido: `WIZ ENTER skill=16
        // target=-1` y ninguna animacion. Pasamos el indice del heroe, que es lo
        // que el skill hace igual: castearse sobre uno mismo.
        int heroIdx = (DAT_07abf5d8 && CharactersClient)
            ? (int)(((uintptr_t)DAT_07abf5d8 - (uintptr_t)CharactersClient) / 916)
            : -1;
        if (heroIdx < 0 || heroIdx >= 400) return;
        Combat_SeedRuntimeState97k(iType, heroIdx);
        UseSkillWizard_stub((DWORD)entity, (DWORD)entity);
        return;
    }

    // 15) iType == 16 (Mana Shield on a party member) — packet 0x19
    if (iType == 16 && SelectedCharacter != -1) {
        char* partyTarget = (char*)(uintptr_t)CharactersClient + 916 * (int)SelectedCharacter;
        if (*(unsigned char*)(partyTarget + 132) != 1 || PartyNumber <= 0) {
            return;
        }
        const char* targetName = partyTarget + 449;
        size_t targetNameLen = strlen(targetName);
        if (targetNameLen == 0) targetNameLen = 1;
        bool isPartyMember = false;
        for (int partyIndex = 0; partyIndex < PartyNumber; ++partyIndex) {
            if (strncmp((const char*)Party + partyIndex * 36,
                        targetName, targetNameLen) == 0) {
                isPartyMember = true;
                break;
            }
        }
        if (!isPartyMember) {
            return;
        }
        *(unsigned short*)(entity + 784) = (unsigned short)SelectedCharacter;
        int tgtGX = (int)DAT_07e016c0;
        int tgtGY = (int)DAT_07e016c4;
        int heroGX = *(int*)(entity + 904);
        int heroGY = *(int*)(entity + 908);
        int skillRange = Combat_GetSkillRange97k(iType);
        float tx = (float)tgtGX * 100.0f + 50.0f;
        float ty = (float)tgtGY * 100.0f + 50.0f;
        float dx = *(float*)(entity + 16) - tx;
        float dy = *(float*)(entity + 20) - ty;
        float dist2 = dx * dx + dy * dy;
        float maxDist = (float)skillRange * 100.0f;
        if (dist2 <= maxDist * maxDist) {
            Combat_SeedRuntimeState97k(iType, (int)SelectedCharacter);
            UseSkillWizard_stub((DWORD)entity, (DWORD)entity);
        } else if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                (unsigned char*)(entity + 852), (float)skillRange) != 0) {
            entity[748] = 1;
            entity[749] = 5;
        }
        return;
    }

    // Attack.c case 48 (Greater Life): siempre es un self-cast. En
    // particular, an existing selected character must not replace HeroKey.
    // Tanto el envío como el camino de cooldown terminan con la acción 63 y limpian el
    // pending movement flag (IDA labels 771/811).
    if (iType == 48) {
        DWORD now = GetTickCount();
        if ((DWORD)(now - g_dwLatestMagicTick_Attack) > 300) {
            unsigned short heroKey = *(unsigned short*)(entity + 476);
            if (heroKey != 0xFFFF) {
                g_dwLatestMagicTick_Attack = now;
                DAT_05826d10 = (DWORD)iType;
                BYTE pkt[6] = { 0xC1, 6, 0x19, (BYTE)iType,
                                (BYTE)(heroKey >> 8), (BYTE)heroKey };
                Net_SendSmallPacket(pkt, sizeof(pkt));
            }
        }
        FUN_0043e820((int)(intptr_t)entity, 63);
        entity[748] = 0;
        return;
    }

    // Attack.c case 55 (Fire Slash): native 0.97k builds a duration packet,
    // not a targeted 0x19 packet. MuEmu's corresponding C3:1E form carries
    // la dirección explícita, el delta de destino clampeado y la tecla elegida.
    if (iType == 55) {
        // El case 55 de 0049CBF0 arranca con la misma actualización de ángulo por objetivo chequeado
        // que el case 56; el paquete de duración y el visual tienen que usar ese ángulo.
        Combat_AimAtCheckedTarget97k(entity, bHasTarget);
        const int heroGX = *(int*)(entity + 904);
        const int heroGY = *(int*)(entity + 908);
        const int targetGX = (int)DAT_07e016c0;
        const int targetGY = (int)DAT_07e016c4;
        const int skillRange = Combat_GetSkillRange97k(iType);
        const float dx = *(float*)(entity + 16) - (targetGX * 100.0f + 50.0f);
        const float dy = *(float*)(entity + 20) - (targetGY * 100.0f + 50.0f);
        const float maxDistance = skillRange * 100.0f;
        if (dx * dx + dy * dy > maxDistance * maxDistance) {
            return;
        }

        int deltaX = targetGX - heroGX;
        int deltaY = targetGY - heroGY;
        if (deltaX < -8) deltaX = -8;
        if (deltaX > 7)  deltaX = 7;
        if (deltaY < -8) deltaY = -8;
        if (deltaY > 7)  deltaY = 7;
        const BYTE destination = (BYTE)(((deltaX + 8) << 4) | ((deltaY + 8) & 0x0F));
        const BYTE direction = (BYTE)((int)(*(float*)(entity + 36) * (256.0f / 360.0f)));

        WORD targetKey = 0xFFFF;
        if (SelectedCharacter != -1 && FUN_00483160() != 0) {
            targetKey = *(unsigned short*)(CharactersClient + 916 * (int)SelectedCharacter + 476);
        }
        Combat_SendDurationSkill97k(entity, iType, 61, direction, destination, 0, targetKey);
        return;
    }

    // Attack.c case 56 (Power Slash): mismo transporte de duración y empaquetado de destino
    // que Fire Slash, con el facing opuesto codificado en el ángulo. IDA
    // termina esta rama con la acción de ataque 81.
    if (iType == 56) {
        // 0049CBF0 case 56 begins with `c+36 = CreateAngle(c+16,c+20,
        // c+788,c+792)`. Esto es distinto de la rama de área 9/12 y hace
        // falta para que el rayo visual y la dirección de C3:1E coincidan.
        Combat_AimAtCheckedTarget97k(entity, bHasTarget);
        const int heroGX = *(int*)(entity + 904);
        const int heroGY = *(int*)(entity + 908);
        const int targetGX = (int)DAT_07e016c0;
        const int targetGY = (int)DAT_07e016c4;
        const int skillRange = Combat_GetSkillRange97k(iType);
        const float dx = *(float*)(entity + 16) - (targetGX * 100.0f + 50.0f);
        const float dy = *(float*)(entity + 20) - (targetGY * 100.0f + 50.0f);
        const float maxDistance = skillRange * 100.0f;
        if (dx * dx + dy * dy > maxDistance * maxDistance) {
            return;
        }

        int deltaX = targetGX - heroGX;
        int deltaY = targetGY - heroGY;
        if (deltaX < -8) deltaX = -8;
        if (deltaX > 7)  deltaX = 7;
        if (deltaY < -8) deltaY = -8;
        if (deltaY > 7)  deltaY = 7;
        const BYTE destination = (BYTE)(((deltaX + 8) << 4) | ((deltaY + 8) & 0x0F));
        const float facing = *(float*)(entity + 36);
        const BYTE direction = (BYTE)((int)(facing * (256.0f / 360.0f)));
        const BYTE angle = (BYTE)((int)((facing + 180.0f) * (256.0f / 360.0f)));

        WORD targetKey = 0xFFFF;
        if (SelectedCharacter != -1 && FUN_00483160() != 0) {
            targetKey = *(unsigned short*)(CharactersClient + 916 * (int)SelectedCharacter + 476);
        }
        Combat_SendDurationSkill97k(entity, iType, 81, direction, destination, angle, targetKey);
        return;
    }

    // Attack.c case 15 tail: Teleport Ally only accepts a visible party member.
    // Elige un tile libre en el 3x3 que rodea al héroe y manda C3:B0.
    if (iType == 15 && SelectedCharacter != -1 && bHasTarget) {
        const int targetIdx = (int)SelectedCharacter;
        char* target = (char*)(uintptr_t)CharactersClient + 916 * targetIdx;
        if (PartyNumber <= 0 || DAT_07e91388 > 0) {
            return;
        }
        const char* targetName = target + 449;
        size_t targetNameLen = strlen(targetName);
        if (targetNameLen == 0) targetNameLen = 1;
        bool isPartyMember = false;
        for (int partyIndex = 0; partyIndex < PartyNumber; ++partyIndex) {
            if (strncmp((const char*)Party + partyIndex * 36,
                        targetName, targetNameLen) == 0) {
                isPartyMember = true;
                break;
            }
        }
        if (!isPartyMember) {
            return;
        }

        const bool isPlayer = (*(unsigned short*)(target + 2) == 390);
        const BYTE targetAnim = *(unsigned char*)(target + 261);
        if ((isPlayer && targetAnim == 87) || (!isPlayer && targetAnim == 5)) {
            return;
        }
        const BYTE targetState = *(unsigned char*)(target + 124);
        if (targetState == 1 || targetState == 2 || *(float*)(target + 360) < 0.7f) {
            return;
        }

        extern unsigned char* TerrainWall;
        const int heroGX = (int)(*(float*)(entity + 16) * 0.01f);
        const int heroGY = (int)(*(float*)(entity + 20) * 0.01f);
        for (int attempt = 0; attempt <= 10; ++attempt) {
            const int x = heroGX + (rand() % 3) - 1;
            const int y = heroGY + (rand() % 3) - 1;
            if (x == heroGX && y == heroGY) continue;
            BYTE terrainAttr = TerrainWall[((y & 0xFF) << 8) | (x & 0xFF)];
            if ((terrainAttr & 0x20) != 0) terrainAttr &= ~0x20;
            if (terrainAttr != 0) continue;

            *(float*)(target + 36) = FUN_0043e050(
                *(float*)(target + 16), *(float*)(target + 20),
                *(float*)(target + 788), *(float*)(target + 792));
            Combat_SeedRuntimeState97k(iType, targetIdx);
            Combat_SendPartyRecall97k(entity, targetIdx, (BYTE)x, (BYTE)y);
            return;
        }
        return;
    }

    // 16b) iType == 47 (DK Death Stab) — gated by CheckAttack ────────────────────────────────────────────────
    if (iType == 47) {
        if (SelectedCharacter == -1 || !bHasTarget) return;
        if (FUN_00483160() == 0) return;  // CheckAttack failed — busy
        int tgtGX = (int)DAT_07e016c0;
        int tgtGY = (int)DAT_07e016c4;
        int heroGX = *(int*)(entity + 904);
        int heroGY = *(int*)(entity + 908);
        int skillRange = Combat_GetSkillRange97k(iType);
        float tx = (float)tgtGX * 100.0f + 50.0f;
        float ty = (float)tgtGY * 100.0f + 50.0f;
        float dx = *(float*)(entity + 16) - tx;
        float dy = *(float*)(entity + 20) - ty;
        float dist2 = dx * dx + dy * dy;
        float maxDist = (float)skillRange * 1.2f * 100.0f;
        if (dist2 <= maxDist * maxDist) {
            Combat_SeedRuntimeState97k(iType, (int)SelectedCharacter);
            FUN_00485780((int)entity, (int)entity);
        } else if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                (unsigned char*)(entity + 852), (float)skillRange * 1.2f) != 0) {
            entity[748] = 1;
            entity[749] = 5;
        }
        return;
    }

    // 17) Heal / buff de Defensa / de Ataque (51) — el mismo camino de helper que Action() ──
    if (iType == 51) {
        if (SelectedCharacter == -1 || !bHasTarget) return;
        int tgtGX = (int)DAT_07e016c0;
        int tgtGY = (int)DAT_07e016c4;
        int heroGX = *(int*)(entity + 904);
        int heroGY = *(int*)(entity + 908);
        int skillRange = Combat_GetSkillRange97k(iType);
        float tx = (float)tgtGX * 100.0f + 50.0f;
        float ty = (float)tgtGY * 100.0f + 50.0f;
        float dx = *(float*)(entity + 16) - tx;
        float dy = *(float*)(entity + 20) - ty;
        float dist2 = dx * dx + dy * dy;
        float maxDist = (float)skillRange * 100.0f;
        if (dist2 <= maxDist * maxDist && FUN_004830b0(heroGX, heroGY, tgtGX, tgtGY)) {
            Combat_SeedRuntimeState97k(iType, (int)SelectedCharacter);
            FUN_0048a180((int)entity, (int)entity);
        } else if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                (unsigned char*)(entity + 852), (float)skillRange) != 0) {
            entity[748] = 1;
            entity[749] = 5;
        }
        return;
    }

    // Los scrolls de invocación del Dark Wizard (30..36) apuntan al propio caster.
    // Attack.c deja este rango fuera de la rama de seguir-al-objetivo; el
    // CGSkillAttackRecv de MuEmu después rutea el C3:19 resultante a SkillSummon.
    if (iType >= 30 && iType <= 36) {
        const DWORD now = GetTickCount();
        if ((DWORD)(now - g_dwLatestMagicTick_Attack) <= 300) {
            return;
        }
        const WORD heroKey = *(WORD*)(entity + 476);
        if (heroKey == 0xFFFF) {
            return;
        }
        g_dwLatestMagicTick_Attack = now;
        DAT_05826d10 = (DWORD)iType;
        BYTE pkt[6] = { 0xC1, 6, 0x19, (BYTE)iType,
                        (BYTE)(heroKey >> 8), (BYTE)heroKey };
        Net_SendSmallPacket(pkt, sizeof(pkt));
        FUN_00444a80((int)(intptr_t)entity);
        entity[748] = 0;
        return;
    }

    // Attack.c cases 41/42 (Twisting Slash/Rageful Blow): these are duration
    // casts. The generic wizard helper has no case for either ID, so letting
    // que lleguen ahí, descarta el skill después de actualizar sólo el facing local.
    if (iType == 41 || iType == 42) {
        *(float *)(entity + 36) = FUN_0043e050(
            *(float *)(entity + 16), *(float *)(entity + 20),
            *(float *)(entity + 788), *(float *)(entity + 792));
        const BYTE direction = (BYTE)((int)(*(float*)(entity + 36) * (256.0f / 360.0f)));
        WORD targetKey = 0xFFFF;
        if (SelectedCharacter != -1 && FUN_00483160() != 0) {
            targetKey = *(unsigned short*)(CharactersClient + 916 * (int)SelectedCharacter + 476);
        }
        // IDA usa acá campos de destino/ángulo en cero. Sus colas son LABEL_497
        // (action 61) and LABEL_930 (action 62), respectively.
        Combat_SendDurationSkill97k(entity, iType, (iType == 41) ? 61 : 62,
                                    direction, 0, 0, targetKey);
        return;
    }

    // IDA Attack.c cases 41/42 (L2938/L3537): ambos rotan siempre hacia la
    // posición cacheada del objetivo en c+788/c+792. Movement_Tick en este
    // call site es CreateAngle (FUN_0043e050); no hay gate de clase en IDA.
    if (iType == 41 || iType == 42) {
        *(float *)(entity + 36) = FUN_0043e050(
            *(float *)(entity + 16), *(float *)(entity + 20),
            *(float *)(entity + 788), *(float *)(entity + 792));
    }

    // 18) Remaining high/runtime skills — wizard helper path.
    Combat_SeedRuntimeState97k(iType, (int)SelectedCharacter);
    UseSkillWizard_stub((DWORD)entity, (DWORD)entity);
}

// FUN_0048ba70 @ 0x0048BA70 — CheckArrow(void)
// Chequea si el personaje del jugador es de clase elfo/arco, que necesita flechas.
// Reads CharacterMachine (DAT_07cf1ffc) offsets [0x86] = class, [0x97] = sub-class (DWORD-indexed).
// Devuelve 1 si la clase puede usar flechas y tiene el equipo correcto, 0 si no.
// All anti-tamper hash table operations (MAIN_HASH_CLASS) skipped per project policy.
char __cdecl FUN_0048ba70(void)
{
    // anti-tamper hash table — skipped
    DWORD* pMachine = (DWORD*)DAT_07cf1ffc;
    if (!pMachine) return 0;

    short charClass = (short)pMachine[0x86];   // CharacterMachine[0x86] = class
    short subClass  = (short)pMachine[0x97];   // CharacterMachine[0x97] = sub-class

    // Elf bow classes: 0x88..0x8E, 0x90, 0x92
    if ((charClass >= 0x88 && charClass <= 0x8E) || (charClass >= 0x90 && charClass <= 0x90) || charClass == 0x92) {
        // Estas clases usan flechas por naturaleza — chequear la subclase para la ballesta
        if (subClass == 0x87 && *(char*)((char*)pMachine + 0x276) == '\0') {
            // Missing crossbow bolt item
            UIChatLogWindow_AddText((const char*)&DAT_07e11df4, (const char*)&DAT_07d3c348, 2);
            FUN_0048b680();
            // anti-tamper hash table — skipped
            return 0;
        }
        // anti-tamper hash table — skipped
        return 1;
    }

    // Clases no-elfo que pueden equipar arcos vía subclase: 0x80..0x86, 0x91
    if ((subClass >= 0x80 && subClass <= 0x86) || subClass == 0x91) {
        // Chequea si es la clase 0x8F (elf master) con el flag de equipo especial
        if (charClass == 0x8F && *(char*)((char*)pMachine + 0x232) != '\0') {
            // anti-tamper hash table — skipped
            return 1;
        }
        // No arrows equipped — display error
        UIChatLogWindow_AddText((const char*)&DAT_07e11df8, (const char*)&DAT_07d3c348, 2);
        FUN_0048b680();
        // anti-tamper hash table — skipped
        return 0;
    }

    // anti-tamper hash table — skipped
    return 1;
}

// ──────────────────────────────────────────────────────────────────────────
// LEGACY STUB — Send_MovePacket_Player_legacy_stub(void)
//
// Originalmente este stub vivía en la dirección 0x0048D640 con el nombre
// "Send_MovePacket_Player". El IDA companion confirma que la función real en
// 0x0048D640 se llama "Action" (despachador de acciones — pickup/equip/attack/
// skill — basado en `*(c+749)` queue), NO un sender de packet 0x10.
//
// Sin embargo, los call-sites existentes de FUN_0048d640() en nuestro codebase
// (5 en Player_InputTick, 1 en stubs/UseSkillWarrior) fueron escritos contra
// este stub y dependen de su comportamiento de "enviar packet 0x10 cuando el
// pathfind falla / cancela". Cambiar FUN_0048d640 al Action real rompería
// el envío de move-packets en runtime.
//
// Estrategia: este stub mantiene su comportamiento original (renombrado para
// que callers apunten a él explícitamente). FUN_0048d640 abajo es ahora el
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
    FUN_00491c40((int)(intptr_t)entity, (int)(intptr_t)entity);
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

    // Clave XOR (la misma que FUN_00491c40 / el paquete de login)
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
        return 4;

    BYTE* rec = nullptr;
    if (DAT_07cf1ff8 != 0)
        rec = (BYTE*)(uintptr_t)DAT_07cf1ff8 + skillType * 0x28;
    else if (DAT_07d29d20 != 0)
        rec = (BYTE*)(uintptr_t)DAT_07d29d20 + skillType * 0x28;

    if (!rec)
        return 4;

    int range = (int)rec[0x27];
    return (range > 0) ? range : 4;
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

static void Combat_SendDurationSkill97k(char* entity, int skillType, int action,
                                        BYTE direction, BYTE destination,
                                        BYTE angle, WORD targetKey,
                                        int packetX, int packetY)
{
    // MuEmu: PMSG_DURATION_SKILL_ATTACK_RECV { C3:1E, skill, x, y,
    // dir, dis, angle, index[2] }.  -1 is the protocol's ground/no-target
    // centinela, tal como lo usa CPatchs::SendRequestMagicContinue.
    BYTE pkt[11];
    pkt[0] = 0xC1;
    pkt[1] = 11;
    pkt[2] = 0x1E;
    pkt[3] = (BYTE)skillType;
    pkt[4] = (BYTE)((packetX >= 0) ? packetX : *(DWORD*)(entity + 904));
    pkt[5] = (BYTE)((packetY >= 0) ? packetY : *(DWORD*)(entity + 908));
    pkt[6] = direction;
    pkt[7] = destination;
    pkt[8] = angle;
    pkt[9] = (BYTE)(targetKey >> 8);
    pkt[10] = (BYTE)targetKey;
    Net_SendSmallPacket(pkt, sizeof(pkt));

    if (action < 0) {
        FUN_00444a80((int)(intptr_t)entity); // Attack.c LABEL_1430
    } else {
        FUN_00443e70();
        FUN_0043e820((int)(intptr_t)entity, action);
        entity[748] = 0;
    }
}

// C3:1D es la segunda mitad de un casteo multi-objetivo. El
// FUN_00485780/0045FDB0 de IDA arma este paquete después de su pedido de duración: el
// cliente enumera su tabla de personajes visibles y aporta los IDs de objeto del
// server. El server sigue siendo la autoridad sobre el radio real del skill y
// target validity (CGMultiSkillAttackRecv -> CheckSkillRadio).
//
// Layout recovered from FUN_00485780:
//   C1 size 1D skill x y serial count [target-id big-endian] * count
// Conservamos a propósito los predicados originales de la tabla que hacen falta para los monstruos:
// active (+0), drawable (+352), alive (!+765), category monster (+132 == 2).
// Cinco objetivos como máximo es lo legal tanto en el builder nativo como en el server.
static void Combat_SendAreaHits97k(char* entity, int skillType)
{
    if (!entity || !CharactersClient)
        return;

    WORD targetIds[5] = {};
    int count = 0;
    const char* table = (const char*)CharactersClient;
    // Skill.txt for the supported no-target skills (Evil Spirit, Hell Fire,
    // Aqua Beam, Inferno) define Radio=6. El server usa ese mismo gate circular en
    // CGMultiSkillAttackRecv; aplicarlo acá replica el selector acotado nativo
    // en vez de reenviar cada mob del viewport.
    constexpr float kRadioWorld = 600.0f;
    const float heroX = *(const float*)(entity + 16);
    const float heroY = *(const float*)(entity + 20);
    // Aqua Beam es la variante que viaja hacia adelante: su dirección C3:1E se
    // deriva de Object.Angle.z, y sus contactos son los círculos de un tile
    // que cruza ese rayo (el mismo radio de contacto de 100 unidades que usa
    // sub_45FDB0 de IDA). No es el círculo centrado en el caster que usan los otros
    // no-target spells.
    const float heading = *(const float*)(entity + 36) * 0.017453292f;
    const float forwardX = sinf(heading);
    const float forwardY = -cosf(heading);

    for (int slot = 0; slot < 400 && count < 5; ++slot) {
        const char* candidate = table + 916 * slot;

        // Rama exacta de monstruos de sub_45FDB0. Los candidatos jugador/PvP usan su
        // cláusula aparte de misma-clave-de-objetivo y a propósito no son parte de
        // these no-target monster casts.
        if (!candidate[0] || !candidate[352] || candidate == entity ||
            candidate[765] || (BYTE)candidate[132] != 2) {
            continue;
        }

        const WORD id = *(const WORD*)(candidate + 476);
        if (id == 0 || id == 0xFFFF)
            continue;

        const float dx = *(const float*)(candidate + 16) - heroX;
        const float dy = *(const float*)(candidate + 20) - heroY;
        if (skillType == 12) {
            const float along = dx * forwardX + dy * forwardY;
            const float across = dx * forwardY - dy * forwardX;
            if (along < 0.0f || along > kRadioWorld || fabsf(across) > 100.0f)
                continue;
        } else {
            if ((dx * dx + dy * dy) > (kRadioWorld * kRadioWorld))
                continue;
        }

        targetIds[count++] = id;
    }

    // There is no valid payload without a target list.  Sending nothing is
    // que es también lo que hace el builder original cuando su selección acotada queda vacía.
    if (count == 0)
        return;

    BYTE pkt[8 + 2 * 5] = {};
    const BYTE serial = *(const BYTE*)(entity + 136); // native o+136 field
    pkt[0] = 0xC1;
    pkt[1] = (BYTE)(8 + count * 2);
    pkt[2] = 0x1D;
    pkt[3] = (BYTE)skillType;
    pkt[4] = (BYTE)*(DWORD*)(entity + 904);
    pkt[5] = (BYTE)*(DWORD*)(entity + 908);
    pkt[6] = serial;
    pkt[7] = (BYTE)count;
    for (int i = 0; i < count; ++i) {
        pkt[8 + 2 * i] = (BYTE)(targetIds[i] >> 8);
        pkt[9 + 2 * i] = (BYTE)targetIds[i];
    }
    Net_SendSmallPacket(pkt, pkt[1]);

    char trace[128];
    wsprintfA(trace, "SKILL SEND C1:1D skill=%u targets=%d serial=%u",
              (unsigned)(BYTE)skillType, count, (unsigned)serial);
    DbgLogPublic(trace);
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
// FUN_0048d640 @ 0x0048D640 — Action(DWORD c, DWORD o)
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
//  - Casos 0,1,3 (pickup/equip/walk): STUB documentado — el cliente original
//    arma packets de pickup/talk-to-npc inline. Dejados como TODO porque la
//    runtime test surface está bloqueada por servidor combat support.
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
//   PathFinding2            FUN_0043f3e0 — pathfind helper (returns 0/1)
//   Movement_Tick           FUN_0043e050 — atan2-based facing-toward (CreateAngle)
//   CheckWall               FUN_004830b0 — line-of-sight tile check (PathRange_Check)
//   UseSkillWarrior         FUN_00485780
//   UseSkillElf             FUN_0048a180
//   SetPlayerStop           FUN_004430c0 — sets idle anim
//   PlayBuffer              FUN_00404bc0 — sound effect by id
//   SetAction               FUN_0043e820 — set entity action
//   sub_4889D0              FUN_004889d0 — skill-attack-finalize helper
//   Net_SendSmallPacket     project helper (Net.h) — does C3 wrap + chain XOR +
//                                                    CSimpleModulus + serial
// ──────────────────────────────────────────────────────────────────────────

// 2026-06-17: dejar de rutear Elf→Warrior. El proyecto ya tiene
// `SkillElf_stub` y ése es el helper correcto para los skills de soporte/elf
// dentro de este branch de combate.
void __cdecl FUN_0048a180(int c, int o) {
    (void)o;
    if (DAT_07cf1ff4 != 0) {
        if (SkillElf_stub((DWORD)c, (DWORD)DAT_07cf1ff4))
            return;
    }
    // El flujo original de elfo/soporte no cae al melee de guerrero si falla.
}

// Declaraciones adelantadas de los helpers que usa Action — casi todos ya están declarados en
// functions.h con los mismos prototipos; los re-declaramos localmente para evitar
// implicit-decl warnings if a particular helper hasn't been wired up yet.
//
// FUN_004889d0 (UseSkillWizard) está portada como UseSkillWizard_stub en
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

void __cdecl FUN_0048d640(DWORD c, DWORD o)
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
    // CASE 0 (action_id=1) — Pickup ground item
    // Original sends packet 0x22 [tgtItemKey] (ground item request) with
    // construcción anti-tamper del paquete. Diferido — el camino de combate del server es la
    // prioridad. El camino de UseGroundItem ya existe en los stubs como
    // FUN_0048b680 + companion handlers.
    // ──────────────────────────────────────────────────────────────────────────
    case 0: {
        // 2026-05-07: port FIEL desde server source
        // Mu-linux-97K/Source/MuServer/GameServer/ItemManager.h:25
        //   struct PMSG_ITEM_GET_RECV {
        //     PBMSG_HEAD header;  // C1:22
        //     BYTE index[2];      // big-endian item key
        //   };
        //
        // Wire: [C1][05][22][ItemKeyH][ItemKeyL] + chain XOR + MuEmu byte XOR.
        // El target item key viene de SelectedItem (set por click handler en
        // Player_InputTick cuando user click on ground item).
        FUN_004430c0((int)c);
        *(unsigned char*)(c + 748) = 0;

        int itemSlotIdx = (int)SelectedItem;
        if (itemSlotIdx >= 0 && itemSlotIdx < 1000) {
            // 2026-07-27 BUG-FIX: el port anterior leía el key del pool de
            // personajes (DAT_07abf5d0, stride 0x394) → basura. Los items del
            // suelo están en DAT_07e12840 (stride 0x204), y el 0x20 viewport
            // handler los guarda en pool[key*0x204] → el ÍNDICE del slot ES el
            // map item index que el server espera en el 0x22 (CGItemGetRecv).
            BYTE* itemEnt = (BYTE*)&DAT_07e12840[0] + (uintptr_t)itemSlotIdx * 0x204;
            if (itemEnt[72]) {   // active
                unsigned short itemKey = (unsigned short)itemSlotIdx;
                BYTE pkt[6];
                pkt[0] = 0xC1; pkt[1] = 0x05; pkt[2] = 0x22;
                pkt[3] = (BYTE)((itemKey >> 8) & 0xFF);
                pkt[4] = (BYTE)(itemKey & 0xFF);
                Net_SendSmallPacket(pkt, 5);
            }
        }
        *(unsigned char*)(c + 749) = 0;
        return;
    }

    // ──────────────────────────────────────────────────────────────────────────
    // CASE 1 (action_id=2) — NPC interact / shop open
    // Original construye packets 0x30 (talk-to-npc) y 0x36 (shop list req).
    // Deferred — UI flow ya está parcialmente cubierto por Game_EnterWorldTick
    // y UI_InGameMenu en otros caminos.
    // ──────────────────────────────────────────────────────────────────────────
    case 1: {
        // 2026-05-07: port FIEL desde IDA Action.c case 1 + server source
        // Mu-linux-97K/Source/MuServer/GameServer/NpcTalk.h:10
        //   struct PMSG_NPC_TALK_RECV {
        //     PBMSG_HEAD header;  // C1:30
        //     BYTE index[2];      // big-endian NPC entity index
        //   };
        //
        // Wire: [C1][05][30][NpcH][NpcL]  + chain XOR + MuEmu byte XOR.
        // El target NPC viene de DAT_00559c70 (set por click handler en
        // Player_InputTick cuando user click on NPC con SelectedNpc válido).
        FUN_004430c0((int)c);                    // SetPlayerStop (local idle anim)
        *(unsigned char*)(c + 748) = 0;

        int npcSlotIdx = (int)DAT_00559c70;
        if (npcSlotIdx >= 0 && npcSlotIdx < 400) {
            char* npcEnt = (char*)(uintptr_t)DAT_07abf5d0 + npcSlotIdx * 0x394;
            unsigned short npcKey = *(unsigned short*)(npcEnt + 0x1DC);
            if (npcKey != 0xFFFF) {
                // Build NPC talk packet [C1][05][30][NpcH][NpcL]
                BYTE pkt[6];
                pkt[0] = 0xC1;
                pkt[1] = 0x05;
                pkt[2] = 0x30;
                pkt[3] = (BYTE)((npcKey >> 8) & 0xFF);
                pkt[4] = (BYTE)(npcKey & 0xFF);
                Net_SendSmallPacket(pkt, 5);
            }
        }
        *(unsigned char*)(c + 749) = 0;          // clear queue
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

        // Adjust attack Range based on equipped weapon (LOWORD class+weapon ids)
        int charClass = *(short*)(c + 536);   // CharacterMachine.Class
        int weaponR   = *(short*)(c + 604);   // CharacterMachine.WeaponR

        if (charClass >= 96 && charClass < 128) Range = 2.2f;        // 2H sword
        if ((charClass >= 136 && charClass < 143) ||
            (charClass >= 144 && charClass < 145) ||
             charClass == 146) Range = 6.0f;                          // bow
        if ((weaponR >= 128 && weaponR < 135) || weaponR == 145) Range = 6.0f; // bow R
        if (*(unsigned char*)(c + 747) == 9) Range = 7.0f;            // polearm

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
                if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
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
    // Stub: dejamos el legacy stub manejar el packet 0x10. Sólo limpiamos queue.
    // ──────────────────────────────────────────────────────────────────────────
    case 3: {
        // 2026-05-08: port FIEL completo de IDA Action.c L1417-2243.
        //
        // 1) Gate de distancia: toma el eje mayor de abs(heroGrid - target). Si
        //    es > 1 tile, el héroe todavía no llegó — retorna y espera.
        // 2) World+tile-id (dword_7DB8708) dispatch — sets v307/v309 flags
        //    y opcionalmente pisa el facing del héroe con flt_7E118E4 antes
        //    sending the move packet.
        // 3) Special-case cave entrance (World 8, tile 38): different packet
        //    (acción 110) y SetAction(137/138). Retorna temprano.
        // 4) Default: send move packet [0xC1][len][0x10][TgtX][TgtY][heading]
        //    [wpCount][wpX[]][wpY[]] via legacy helper.
        // 5) After move packet:
        //    - If v309: send anim packet [0xC1][0x06][0x18][0x01][heading][109]
        //      with SetAction(139/140) — short-distance teleport (Devias hidden
        //      plaza, Atlans portal, etc.)
        //    - If v307: send anim packet [0xC1][0x06][0x18][0x01][heading][108]
        //      with SetAction(133/135) — gate teleport (Lorencia→Dungeon, etc.)

        // ── 1. Distance gate ──────────────────────────────────────────────────────────────────────────
        int heroGX = *(int*)(c + 904);
        int heroGY = *(int*)(c + 908);
        // En nuestro build TargetX/Y no son globals — el destino se guarda en
        // la entidad del jugador en +0x306/0x307 (target_grid_x/y, lo setea el click
        // handler in Player_InputTick).
        int TargetX_v = (int)*(unsigned char*)(o + 0x306);
        int TargetY_v = (int)*(unsigned char*)(o + 0x307);
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
        case 3:  // Noria — no special tiles
            break;
        case 7:  // Atlans
            if (TileSub == 39) {            overrideFacing=true; v309 = true; }
            break;
        case 8:  // Tarkan / Lost Tower entry
            if (TileSub == 8 || TileSub == 78) {                v307 = true; }
            else if (TileSub == 38) {
                // ── 3. Cave entrance (Lost Tower) — special early return ??????
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
        }

        if (overrideFacing) {
            *(float*)((char*)(uintptr_t)DAT_07abf5d8 + 36) = *(float*)&_DAT_07e118e4;
        }

        // ── 4. Send move packet (always, unless cave returned above) ───────────────────────────
        FUN_00491c40((int)c, (int)o);
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
        // Resolve skill type
        int skillType = (int)Combat_ResolveQueuedSkillId97k();

        int skillTarget = (int)DAT_07d780a0;  // MovementSkillTarget
        if (skillTarget < 0) {
            DbgLogPublic("SKILL TARGET: rejected: no MovementSkillTarget");
            return;
        }
        char* pTgt = ACTION_CHARS_CLIENT + 916 * skillTarget;
        if (*(unsigned char*)(pTgt + 765) != 0) {
            DbgLogPublic("SKILL TARGET: rejected: target marked dead");
            return;
        }

        int tgtGX = (int)(*(float*)(pTgt + 16) * 0.01f);
        int tgtGY = (int)(*(float*)(pTgt + 20) * 0.01f);

        int skillRange = Combat_GetSkillRange97k(skillType);
        Combat_SeedRuntimeState97k(skillType, skillTarget);

        float dx = *(float*)(o + 16) - ((float)tgtGX * 100.0f + 50.0f);
        float dy = *(float*)(o + 20) - ((float)tgtGY * 100.0f + 50.0f);
        float dist = dx*dx + dy*dy;
        float rng  = (float)skillRange * 100.0f;
        bool inRange = (dist <= rng * rng);
        bool forceWalk = (*(unsigned char*)(c + 846) != 0);

        {
            char trace[192];
            wsprintfA(trace,
                "SKILL TARGET: type=%d slot=%d key=%u range=%d inRange=%d safe=%d dist2=%.1f",
                skillType, skillTarget, (unsigned)*(unsigned short*)(pTgt + 476),
                skillRange, inRange ? 1 : 0, forceWalk ? 1 : 0, dist);
            DbgLogPublic(trace);
        }

        // Lee la posición de grilla del héroe (encriptada, pero en nuestro build es un DWORD plano)
        int heroGX = *(int*)(c + 904);
        int heroGY = *(int*)(c + 908);

        auto seedRuntimeTarget = [&]() {
            Combat_SeedRuntimeState97k(skillType, skillTarget);
        };

        switch (skillType) {

        // ─── Direct-target skills (Falling Slash, Lunge, Uppercut, Cyclone,
        //                          Slash, Triple Shot, Sleep) ??????
        case 1: case 2: case 3: case 4: case 7: case 11: case 17:
            if (inRange && !forceWalk) {
                if (FUN_004830b0(heroGX, heroGY, tgtGX, tgtGY)) {
                    // Direct-target wizard path (UseSkillWizard @ 004889D0).
                    seedRuntimeTarget();
                    UseSkillWizard_stub((DWORD)c, (DWORD)o);
                }
            } else {
                if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
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
                if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                 (unsigned char*)(c + 852),
                                 (float)skillRange * 1.2f)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            } else {
                seedRuntimeTarget();
                FUN_00485780((int)c, (int)o);  // UseSkillWarrior
            }
            break;

        // Targeted Elf support skills (Heal, Greater Defense, Greater Damage).
        // El dispatch usa UseSkillElf, que manda C3:19 con este objetivo.
        case 26: case 27: case 28:
            if (inRange && !forceWalk) {
                if (FUN_004830b0(heroGX, heroGY, tgtGX, tgtGY)) {
                    seedRuntimeTarget();
                    FUN_0048a180((int)c, (int)o);  // UseSkillElf (AOE)
                } else if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                        (unsigned char*)(c + 852), (float)skillRange)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            } else {
                if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                 (unsigned char*)(c + 852), (float)skillRange)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            }
            break;

        // El skill 51 sigue el mismo dispatch de alcance / línea de vista en IDA.
        case 51:
            if (inRange && !forceWalk) {
                if (FUN_004830b0(heroGX, heroGY, tgtGX, tgtGY)) {
                    seedRuntimeTarget();
                    FUN_0048a180((int)c, (int)o);
                } else if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
                                        (unsigned char*)(c + 852), (float)skillRange)) {
                    *(unsigned char*)(c + 748) = 1;
                }
            } else {
                if (FUN_0043f3e0(heroGX, heroGY, tgtGX, tgtGY,
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

