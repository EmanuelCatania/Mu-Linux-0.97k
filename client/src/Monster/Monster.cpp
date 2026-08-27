// Monster.cpp
// Sistema de monstruos / NPCs — spawn, modelo, estado, flags de efectos
//
// Cubre:
//   Entity_FindById          @ 0x0045ac80  (19 líneas)
//   Entity_FindByIdAndType   @ 0x0045bfa0  (32 líneas)
//   Entity_FindOrSpawn       @ 0x0045ccf0  (2891 líneas — gran switch de tipos)
//   Entity_Init              @ 0x0045adc0  (797 líneas)
//   Entity_ClearMesh         @ 0x00449840  (51 líneas)
//   Monster_LoadModel        @ 0x005098c0  (542 líneas)
//   BMD_Load                 @ 0x005060b0  (31 líneas) — carga .bmd
//   BMD_LoadCompressed       @ 0x00442a60  (166 líneas) — carga .bmd (datos)
//   Entity_SetFlags          @ 0x0043bde0  (86 líneas) — aplica estado/efectos
//   Entity_ClearFlag         @ 0x0043c070  (75 líneas) — quita flag de estado
//   PacketHandler_0x1f       @ 0x0042a530  (322 líneas) — opcode 0x1f (entity list)
//
// ── ENTITY ARRAY ─────────────────────────────────────────────────────────────
//
//   Base:  DAT_07abf5d0
//   Stride: 0x394 (916 bytes) por entidad
//   Count:  400 entidades máximo (índice 0..399)
//   Slot vacío: entity[+0x00] == 0
//
//   Campos de identificación:
//     +0x00 (byte)  active           — 0 = slot libre, != 0 = en uso
//     +0x02 (short) entity_type      — tipo de entidad (ver tabla abajo)
//     +0x1dc(short) entity_id        — ID único de red (server-assigned)
//     +0x1c1(char[])entity_name      — nombre del monstruo/NPC (string)
//
// ── ENTITY TYPES (entity[+0x02]) ─────────────────────────────────────────────
//
//   Range      | Descripción
//   ───────────┼───────────────────────────────────────────────────────────────
//   0x10E      | Monstruo genérico (default en Entity_FindByIdAndType)
//   0x10F      | NPC tipo 1 / Walking NPC (move_type 1)
//   0x110      | NPC tipo 2 (move_type 2)
//   0x117      | NPC tipo 3 (move_type 3)
//   0x186      | Player (DK/DW/Elf/MG — entity usado en Login, Combat, etc.)
//   0x145      | Objeto especial (excluido de cierto efecto en Entity_SetFlags)
//
//   Entity_FindOrSpawn asigna el tipo según el campo move_type del paquete:
//     move_type 0,4,8 → 0x10E (monstruo/NPC estándar)
//     move_type 1     → 0x10F (NPC con IA ambulante)
//     move_type 2     → 0x110 (NPC estático)
//     move_type 3     → 0x117 (NPC tipo 9 = guardas/especiales)
//     move_type 5     → 0x10F (mismo que 1)
//
// ── ENTITY_FINDBYID (0x0045ac80) ──────────────────────────────────────────────
//
//   int Entity_FindById(int entity_id):
//     // Busca en el array lineal (max 400 entradas)
//     for i in 0..399:
//       entity = DAT_07abf5d0 + i * 0x394
//       if entity[0] != 0 && entity[+0x1dc] == entity_id:
//         return i       // índice en el array
//     return 400         // no encontrado (sentinel)
//
//   Usado por: todos los packet handlers de combate
//   Complejidad: O(n) — scan lineal, max 400
//
// ── ENTITY_FINDBYIDANDTYPE (0x0045bfa0) ───────────────────────────────────────
//
//   char* Entity_FindByIdAndType(int entity_id, uint entity_type):
//     // Busca igual que FindById pero por entity_type en +0x1dc
//     for i in 0..399:
//       entity = DAT_07abf5d0 + i * 0x394
//       if entity[0] != 0 && entity[+0x1dc] == entity_id:
//         Entity_Init(entity, entity_type)   // re-inicializa si ya existe
//         return entity_ptr
//
//     // Si no encontrado: busca slot libre y spawna
//     for i in 0..399:
//       if entity[0] == 0:                   // slot vacío
//         Entity_ClearMesh(entity, entity, 0)
//         Entity_Init(entity, entity_type)
//         entity[+0x1dc] = entity_id
//         return entity_ptr
//
//     return DAT_07abf5d0 + 0x59740          // sentinel: puntero al último slot+1
//
// ── ENTITY_FINDORSPAWN (0x0045ccf0) ───────────────────────────────────────────
//
//   char* Entity_FindOrSpawn(uint move_type, uint grid_x, uint grid_y, int entity_id, ushort flags):
//     // Función de 2891 líneas. Switch principal por move_type:
//
//     case 1 (NPC ambulante):
//       Monster_LoadModel(1)               // carga Monster/Monster01.bmd
//       entity = Entity_FindByIdAndType(entity_id, 0x10F)
//       // Nombre: DAT_005599b0 ("Goblin" o similar)
//       // entity[+0x0C..0x0F] = {0x9A, 0x99, 0x59, 0x3F} — (0.85f = color base)
//       // entity[+0x270] = 0x94, entity[+0x271] = 0x01  — weapon slot
//
//     case 2 (NPC estático):
//       Monster_LoadModel(2)
//       entity = Entity_FindByIdAndType(entity_id, 0x110)
//       // Nombre: DAT_005599e0
//       // entity[+0x0C..0x0F] = {0x00, 0x00, 0x00, 0x3F}
//
//     case 3 (guarda/especial):
//       Monster_LoadModel(9)
//       entity = Entity_FindByIdAndType(entity_id, 0x117)
//       // Nombre: DAT_005599e0
//       // entity[+0x0C..0x0F] = {0xCD, 0xCC, 0xCC, 0x3E}
//
//     default (move_type 0,4,5,8 — monstruo estándar):
//       Monster_LoadModel(0)
//       entity = Entity_FindByIdAndType(entity_id, 0x10E)
//
//       switch (move_type):
//         case 0: Goblin-like. name=DAT_0055987c. weapon=(0xB6,0x01). scale={0xCD,0xCC,0x4C,0x3F}
//         case 4: Balrog-like. weapon=(0xF7,0x01). scale=(0x33,0x33,0x93,0x3F)
//         case 5: Same weapon as 1 (0x94,0x01). flags entity[+0x1be]=1
//         case 8: weapon=(0xF8,0x01). same flags as 5
//         (otros: variaciones de colores y armas)
//
//     Después del switch, Entity_FindOrSpawn configura:
//       entity[+0x388] = grid_x (float, cached waypoint X)
//       entity[+0x38c] = grid_y (float, cached waypoint Y)
//       entity[+0x306] = grid_x (byte, target grid X)
//       entity[+0x307] = grid_y (byte, target grid Y)
//       entity[+0x24]  = facing angle (de paquete)
//       world_x = grid_x * tile_size + offset
//       world_y = grid_y * tile_size + offset
//       entity[+0x10]  = world_x
//       entity[+0x14]  = world_y
//
//   Entity_FindOrSpawn es llamado desde PacketHandler_0x13 y PacketHandler_0x1f.
//
// ── ENTITY_INIT (0x0045adc0) ──────────────────────────────────────────────────
//
//   void Entity_Init(byte* entity, uint entity_type):
//     // Función de 797 líneas, mayormente HashTable anti-tamper.
//     // Lógica real:
//
//     // Registra entity[+0x388] (cached_wp_x) en HashTable
//     HashTable_GetIndex(entity + 0x388)
//     if not found: new node, add to table
//     entity[+0x388] = entity_type & 0xFF   // escribe type en low byte
//
//     // Registra entity[+0x38C] (cached_wp_y) en HashTable
//     entity[+0x38C] = entity_type >> 8     // high byte del type
//
//   Propósito: el HashTable registra las entidades activas para tracking
//   anti-tamper. La lógica real es solo actualizar los dos campos de type.
//
// ── ENTITY_CLEARMESH (0x00449840) ─────────────────────────────────────────────
//
//   void Entity_ClearMesh(int param1, int param2, int param3):
//     // Libera recursos de malla 3D asociados a la entidad
//
//     if (param2 != 0 && entity[+0x184] != NULL):
//       count = entity[+0x180]              // número de sub-meshes
//       for i in 0..count-1:
//         FUN_004086e0(entity[+0x184][i * 0x15])  // release sub-mesh i
//       release(entity[+0x184])            // release pointer array
//       entity[+0x184] = NULL
//       entity[+0x180] = 0
//
//     if (param1 != 0):
//       ptr = entity + 500                  // offset 0x1F4
//       for i in 0..5:                      // 6 attach points
//         if ptr[i*6] != NULL:
//           FUN_004086e0(ptr[i*6])           // release attach mesh
//           ptr[i*6] = NULL
//
//     if (param3 != 0 && entity[+0x14] != NULL):
//       FUN_004086e0(entity[+0x14])          // release extra mesh
//       entity[+0x14] = NULL
//
//   entity[+0x180] (byte)  = sub-mesh count
//   entity[+0x184] (ptr[]) = array de punteros a sub-meshes
//   Stride del array de attach points: 6 dwords = 24 bytes
//
// ── MONSTER_LOADMODEL (0x005098c0) ───────────────────────────────────────────
//
//   void Monster_LoadModel(int monster_type):
//     // monster_type = 0..0x2C (44+ tipos documentados)
//     entity_def_idx = monster_type + 0x10E    // índice en DAT_05828d58
//     entity_def = DAT_05828d58 + entity_def_idx * 0xBC
//
//     // Si ya cargado (anim_count > 0): skip
//     if (DAT_0055a7c4 == 0 && entity_def[+0x24] > 0): skip
//
//     // Carga el archivo BMD del monstruo:
//     BMD_Load(entity_def_idx, "Data/Monster/", "Monster", monster_type + 1)
//       → path: "Data/Monster/Monster01.bmd" para monster_type=0
//       → path: "Data/Monster/Monster10.bmd" para monster_type=9
//       → Si < 10: format "Data/Monster/Monster0%d.bmd"
//       → Si >= 10: format "Data/Monster/Monster%d.bmd"
//
//     // Configura velocidades de animación base:
//     entity_def[anim_speed_table][0] = 0.25f   // idle
//     entity_def[anim_speed_table][1] = 0.20f   // walk
//     entity_def[anim_speed_table][2] = 0.34f   // run
//     entity_def[anim_speed_table][3] = 0.33f   // attack
//     entity_def[anim_speed_table][4] = 0.33f   // hurt
//     entity_def[anim_speed_table][5] = 0.50f   // die
//     entity_def[anim_speed_table][6] = 0.55f   // (extra)
//     entity_def[+0x60] = 1                      // loaded flag
//
//     // Ajustes de velocidad por tipo:
//     Si monster_type == 3: speeds *= DAT_00552a1c   // Skeleton: más rápido
//     Si monster_type == 5 || 0x19: speeds *= DAT_00552928  // Poison Bull: lento
//     Si monster_type == 0x25 || 0x2A: speeds *= DAT_005528b4  // especiales
//
//     // Override de velocidad "run" (anim_speed[2]) por tipo:
//     case 2:   0x8B = 0.70f   // Bull Fighter
//     case 6:   0x8C = 0.60f   // Worm
//     case 8:   0x8C = 0.70f   // Skeleton Warrior
//     case 9:   0xCC = 1.20f   // Beast Master
//     case 10:  0x47 = 0.28f   // Lich
//     case 12,28: 0x4C = 0.30f // Poison Golem / similar
//     case 13:  0x47 = 0.28f
//     case 17,21: 0x80 = 0.50f
//     case 19:  0x60 = 0.60f
//     case 20:  0x66 = 0.40f
//     case 23:  0x30 = 0.19f
//     case 25:  0x1C = 0.11f
//     case 27:  0x31 = 0.20f
//     case 29:  0x1C = 0.14f
//     case 37,40: entity_def[+0x10] = 1   // flag especial (levitante?)
//
//     // Configura sound samples por tipo (FUN_00404a10):
//     FUN_00404a10(sound_id, &sound_data, 2, 1) — 5 sonidos por monstruo:
//       0xAA..0xAE  = monstruo tipo 0 (Goblin: idle/walk/attack/hurt/die)
//       0xAF..0xB3  = monstruo tipo 1 (Bull Fighter)
//       0xB4..0xB6,0xBF..0xC2 = tipo 2 (Worm)
//       ... (continúa para todos los 44 tipos)
//
//     // Configura índices de animación (Model_SetAnimationSlots):
//     Model_SetAnimationSlots(entity_def_idx, anim0, anim1, anim2, anim3, anim4)
//       → Asigna idle/walk/run/attack/hurt animation IDs al entity_def
//
//     // Establece max_anim_count en DAT_05828d58 + offset para ese tipo
//
// ── BMD_LOAD (0x005060b0) ─────────────────────────────────────────────────────
//
//   void BMD_Load(int entity_def_idx, char* folder, char* base_name, int num):
//     // Construye path: "folder/base_name[0N].bmd"
//     if num == -1: path = "folder/base_name.bmd"
//     elif num < 10: path = "folder/base_name0%d.bmd"
//     else: path = "folder/base_name%d.bmd"
//
//     if (DAT_0055a7c4 == 0):   // sin compresión
//       if entity_def[+0x22] > 0: skip  // ya cargado
//       FUN_00442a60(entity_def, folder, path_bmd)  // BMD_LoadFile
//     else:
//       FUN_004423e0(entity_def, folder, path_bmd)  // BMD_LoadCompressed
//
//   Directorio "Data/Monster/" contiene los modelos BMD de monstruos.
//   "Data/Player/" contiene modelos de personajes.
//   "Data/Item/" contiene modelos de ítems en suelo.
//
// ── BMD_LOADFILE (0x00442a60) ─────────────────────────────────────────────────
//
//   uint BMD_LoadFile(void* entity_def, char* folder, char* filename):
//     // Construye path = folder + filename
//     fp = fopen(path, "rb")
//     if (!fp): return 0
//
//     fseek(fp, 0, SEEK_END); fsize = ftell(fp); fseek(fp, 0, SEEK_SET)
//     buffer = new byte[fsize]
//     fread(buffer, 1, fsize, fp)
//     fclose(fp)
//
//     // Header (8 bytes):
//     entity_def[+0x28..+0x2C] = *(int*)(buffer + 0)  // vertex_count o version
//     entity_def[+0x2C..+0x30] = *(int*)(buffer + 4)  // face_count
//
//     // Datos de malla (variable):
//     // Escribe en entity_def + offset tablas de vértices, normales, UVs, huesos
//     // entity_def[+0x30] = ptr a data principal de la malla
//
//     delete buffer
//     return 1
//
//   BMD header signature: los primeros bytes identifican la versión del formato.
//   Formato "BMD" (Binary Model Data): propio de Webzen para Mu Online.
//
// ── ENTITY_SETFLAGS (0x0043bde0) ─────────────────────────────────────────────
//
//   void Entity_SetFlags(uint flags, float* entity):
//     // entity[+0x78] = status_flags (int, bitmap)
//     // Aplica efectos visuales + partículas según bits del flag
//
//     bit 0x01: entity[+0x78] |= 1    // generic flag A
//     bit 0x02: entity[+0x78] |= 2    // generic flag B
//     bit 0x04: entity[+0x78] |= 4    // generic flag C
//     bit 0x08: entity[+0x78] |= 8    // generic flag D
//
//     bit 0x10 (Poison buff):
//       Particle_StartLoop(0x47E, entity)
//       Particle_Spawn(0x47E, world_pos, ...)   // partícula de veneno
//       entity[+0x78] |= 0x10
//
//     bit 0x20 (Ice/freeze buff):
//       Si no ya activo (!(flags & 0x20)):
//         Particle_StartLoop(0xBE, entity)
//         Particle_Spawn(0xBE, world_pos, offset_z+0.01f, ...)  // hielo
//         Particle_Spawn(0xBE, world_pos, offset_z+0.01f, ...)  // doble
//       entity[+0x78] |= 0x20
//
//     bit 0x40 (Lightning buff):
//       Si no ya activo:
//         Particle_StartLoop(0x4FA, entity)
//         color = {1.0, 1.0, 1.0}
//         Particle_Spawn(0x4FA, world_pos, ...)  // rayo
//         FUN_00404bc0(0x68, entity, 0)          // UI event 0x68
//       entity[+0x78] |= 0x40
//
//     bit 0x80 (Fire buff):
//       Si no ya activo:
//         Particle_StartLoop(0x4FA, entity, mode=3)
//         Particle_Spawn(0x4FA, ...)             // fuego
//       entity[+0x78] |= 0x80
//
//     bit 0x100 (Skill shield buff):
//       Si no ya activo && entity_type != 0x145:
//         FUN_00404bc0(0x67, 0, 0)               // UI event 0x67
//         Particle_StartLoop(0x10A, entity, mode=0)
//         5× Particle_Spawn(0x10A, ...)          // escudo orbital
//       entity[+0x78] |= 0x100
//
//   Efecto IDs conocidos:
//     0x47E  = Poison cloud
//     0x4FA  = Electric / Fire spark
//     0x0BE  = Ice crystal
//     0x10A  = Shield orbit (5 partículas)
//     0x68   = UI event "electric hit"
//     0x67   = UI event "shield start"
//
// ── ENTITY_CLEARFLAG (0x0043c070) ─────────────────────────────────────────────
//
//   void Entity_ClearFlag(ushort flag, int entity):
//     // Quita bit del bitmap de status y detiene el efecto visual
//     // entity[+0x78] = status_flags (int)
//
//     flag 0x01: entity[+0x78] &= ~1
//     flag 0x02: entity[+0x78] &= ~2
//     flag 0x04: entity[+0x78] &= ~4
//     flag 0x08: entity[+0x78] &= ~8
//                Particle_StopLoop(0x10A, entity, mode=4)  // quita efecto 0x10A
//     flag 0x10: entity[+0x78] &= ~0x10
//                Particle_StopLoop(0x47E, entity)           // quita veneno
//     flag 0x40: entity[+0x78] &= ~0x40
//                Particle_StopLoop(0x4FA, entity, mode=0)   // quita rayo
//     flag 0x80: entity[+0x78] &= ~0x80
//                Particle_StopLoop(0x4FA, entity, mode=3)   // quita fuego
//     flag 0x100: entity[+0x78] &= ~0x100
//                Particle_StopLoop(0x10A, entity)            // quita escudo
//     flag 0x200: entity[+0x78] &= ~0x200
//
//   Llamado desde Net_Process opcode 0x07 (entity state clear).
//
// ── PACKETHANDLER_0X1F (0x0042a530) ───────────────────────────────────────────
//
//   void PacketHandler_0x1f(int pkt):
//     // "Entity list" packet — múltiples entidades en una respuesta
//     count = byte[4]
//     ptr = pkt + 0x0C
//
//     for i in 0..count-1:
//       entity_id = byte[-7]*256 + byte[-6] & 0x7FFF
//       is_mine   = entity_id >> 15      // bit 15 = entidad propia
//       move_type = byte[-5]
//       flags     = byte[-3..-2] (short)
//       angle_packed = byte[3]           // nibble high = dirección
//       grid_x    = byte[-1]
//       grid_y    = byte[0]
//
//       entity = Entity_FindOrSpawn(move_type, grid_x, grid_y, entity_id, 0)
//       Entity_SetFlags(flags, entity)
//
//       // facing angle:
//       entity[+0x24] = (float(angle_packed >> 4) - DAT_0055256c) * DAT_00552844
//
//       entity[+0x1DC] = entity_id         // set network ID
//       entity[+0x21]  = 1                 // is_active = true
//       entity[+0x2EA] = angle_packed & 0xF // direction nibble
//
//       // Si direction > 5: entity[+0x1BE] = 1
//
//       // Copia nombre del monstruo:
//       memcpy(entity[+0x1C1], name_from_pkt, ...)
//       entity[+0x1CB] = 0   // null terminator
//
//       // Extra data (guild/clan mark?):
//       entity[+0x1C1..+0x1CA] = pkt_name[0..9]
//       entity[+0x1C5..+0x1C8] = *(int*)(pkt + 8)
//       entity[+0x1C9..+0x1CA] = *(short*)(pkt + 0xC)
//
//       // Copia nombre de entidad en DAT_07d4d580 (display buffer?)
//
//       stride = variable por move_type (aprox 0x11..0x16 bytes)
//       ptr += stride
//
//   Diferencia con PacketHandler_0x14 (opcode 0x14):
//     0x14 = spawn individual (Entity_FindById + FUN_0045ac20)
//     0x1F = lista de entidades (Entity_FindOrSpawn multiple)
//
// ── STATUS FLAGS (entity[+0x78]) ─────────────────────────────────────────────
//
//   Bit  | Flag  | Nombre        | Visual effect
//   ─────┼───────┼───────────────┼─────────────────────────────────────────────
//   0    | 0x001 | flag_A        | (sin visual)
//   1    | 0x002 | flag_B        | (sin visual)
//   2    | 0x004 | flag_C        | (sin visual)
//   3    | 0x008 | flag_D        | Particle_StopLoop 0x10A modo 4
//   4    | 0x010 | poison        | Particle 0x47E (poison cloud)
//   5    | 0x020 | ice/freeze    | Particle 0x0BE (ice crystal ×2)
//   6    | 0x040 | lightning     | Particle 0x4FA modo 0 (lightning)
//   7    | 0x080 | fire          | Particle 0x4FA modo 3 (fire)
//   8    | 0x100 | skill_shield  | Particle 0x10A ×5 (orbital shield)
//   9    | 0x200 | flag_extra    | (sin visual conocido)
//
// ── ENTITY_DEF TABLE (DAT_05828d58) ──────────────────────────────────────────
//
//   Tabla de definiciones de entity types. Stride: 0xBC (188 bytes) por entry.
//   Base: DAT_05828d58
//   Index: entity_type → entry = DAT_05828d58 + entity_type * 0xBC
//
//   Offsets en la entry (entity_def):
//     +0x22 (short) load_count        — cuántas veces fue cargado (skip si > 0)
//     +0x24 (short) anim_count        — número de animaciones cargadas
//     +0x26 (short) max_anim_id       — ID máximo de animación válida
//     +0x30 (ptr)   bmd_data_ptr      — puntero a datos BMD cargados en heap
//     +0x60 (byte)  loaded_flag       — 1 después de Monster_LoadModel
//     anim_speed_table[] — floats de velocidad por animación (stride 0x10)
//
//   Índices especiales:
//     0x10E = monstruo genérico (monster_type 0)
//     0x10E + N = monster_type N (para N = 0..0x2C)
//     0x186 = Player (personaje jugable)
//
// ── PIPELINE DE SPAWN DE MONSTRUO ────────────────────────────────────────────
//
//   Servidor envía opcode 0x1F (entity list) o 0x14 (entity spawn):
//
//   1. PacketHandler_0x1F / PacketHandler_0x14
//   2.   → Entity_FindOrSpawn(move_type, grid_x, grid_y, entity_id)
//   3.      → Monster_LoadModel(monster_type)       // carga .bmd si no cargado
//   4.         → BMD_Load("Data/Monster/", "Monster", N)
//   5.            → BMD_LoadFile(entity_def, folder, path)  // fopen + fread
//   6.      → Entity_FindByIdAndType(entity_id, entity_type)
//   7.         → Entity_ClearMesh() si reusando slot
//   8.         → Entity_Init(entity, entity_type)    // registra en HashTable
//   9.   → Entity_SetFlags(flags, entity)            // aplica buffs/efectos
//  10.   → entity[+0x1DC..1CB] = network data       // nombre, ID, dirección
//
//   Per-frame (ya documentado en Movement.cpp y Combat.cpp):
//   - PacketHandler_0x10 (0x10 = entity move path)
//   - PacketHandler_0x17 (0x17 = entity death)
//   - PacketHandler_0x12 (0x12 = entity attack)
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   DAT_07abf5d0  — entity array base (stride 0x394, max 400)
//   DAT_07abf5d8  — local player entity ptr
//   DAT_05828d58  — entity_def table (stride 0xBC)
//   DAT_0055a7c4  — compressed_assets flag (0=plain, 1=compressed)
//   DAT_07d4d580  — name display buffer
//
// ── CROSS-REFERENCE ───────────────────────────────────────────────────────────
//
//   Entity_FindById         @ 0x0045ac80  — scan lineal por entity_id
//   Entity_FindByIdAndType  @ 0x0045bfa0  — find + spawn si no existe
//   Entity_FindOrSpawn      @ 0x0045ccf0  — dispatch por move_type (2891 líneas)
//   Entity_Init             @ 0x0045adc0  — registra en HashTable anti-tamper
//   Entity_ClearMesh        @ 0x00449840  — libera meshes del slot
//   Monster_LoadModel       @ 0x005098c0  — carga BMD + configura anim speeds
//   BMD_Load                @ 0x005060b0  — selecciona path y loader
//   BMD_LoadFile            @ 0x00442a60  — fopen + fread del .bmd
//   BMD_LoadCompressed      @ 0x004423e0  — versión comprimida (DAT_0055a7c4==1)
//   Monster_SetAnimIndices  @ 0x00509810  — asigna IDs de anim a entity_def
//   Entity_SetFlags         @ 0x0043bde0  — aplica buff/debuff visual
//   Entity_ClearFlag        @ 0x0043c070  — quita buff/debuff
//   PacketHandler_0x1F      @ 0x0042a530  — entity list spawn
//   PacketHandler_0x14      @ Net_Process opcode 0x14 (FUN_0045ac20)
//   Particle_Spawn          @ 0x00460dc0  (ver Combat.cpp)
//   Particle_StartLoop      @ 0x0046fe00  (ver Particle_Render.cpp)
//   Particle_StopLoop       @ 0x00460d20

#include "stdafx.h"
#include "Monster/Monster.h"

// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 5239-6163 (925 lines)
// FUN_0045bfa0 (CreateCharacter), FUN_0045ccf0 (CreateMonster — big switch)
// =============================================================================
// FUN_0045ccf0 @ 0x0045CCF0 — CreateMonster(Type, PositionX, PositionY, Key, [phantom])
// Ported from IDA Hex-Rays decompile (10619 bytes).
//
// Spawns a monster/NPC entity by Type ID:
//   1. Loads the BMD model via OpenMonsterModel/OpenNpc.
//   2. Calls CreateCharacter to allocate/find an entity slot keyed by Key.
//   3. Writes per-Type scale (+0x0C), weapon item IDs (+624,+648), animation
//      bases (+504..+600), facing/extra (+446), action flags (+100/+104), etc.
//   4. For "world-tier" monsters (84..136 in worlds 9..16) bumps scale by world group.
//   5. Scans MonsterScript table for a matching Type → overrides display name.
//   6. Tags entity_type at +0x2EB (747) and HeroIndex copy at +0x310 (784).
//   7. Sets entity flags byte (+0x84/+132): 2=normal monster, 4=NPC, 8=ground item.
//
// strcpy(c+449, …) calls in the original switch have been omitted — the trailing
// MonsterScript scan overrides the name field anyway, and the original byte_5599xx
// addresses are Korean strings in the data segment we don't reproduce.
//
// CreateCharacter (FUN_0045bfa0) is also implemented here (was a 3-arg stub).
//
// Helpers used (all already implemented in our codebase):
//   FUN_005098c0 (OpenMonsterModel)  — Monster_Data.cpp
//   FUN_0045adc0 (CreateCharacterPointer) — Entity_Spawn.cpp
//   FUN_00449840 (DeleteCloth/Entity_ClearBoneLinks) — stubs.cpp
//   FUN_0045c050 (SetCharacterScale) — alias macro
//   FUN_0043e820 (SetAction)
//   Joint_Create (CreateJoint)
//   FUN_004f7500 (RequestTerrainHeight)
//   OpenNpc_stub (0x005091D0)
extern "C++" {
extern void __cdecl OpenNpc_stub(int Type);
}

// CreateCharacter — finds/allocates an entity slot for Key, returns pointer.
// 1) scan first 400 slots for matching key at +476 → reuse slot
// 2) else find first inactive slot (active flag at +0 == 0) → init it
// Returns pointer (DWORD) into CharactersClient (DAT_07abf5d0).
unsigned int __cdecl FUN_0045bfa0(int Key, int Type, unsigned char PosX,
                                   unsigned char PosY, float Rotation)
{
    unsigned int c = DAT_07abf5d0;
    for (int i = 0; i < 400; ++i) {
        if (*(unsigned char*)c != 0 && *(short*)(c + 476) == (short)Key) {
            FUN_0045adc0((unsigned char*)c, Type, PosX, PosY, Rotation);
            return c;
        }
        c += 916;
    }
    // search again for an inactive slot
    int v7 = 0;
    unsigned int i = DAT_07abf5d0;
    while (*(unsigned char*)i != 0) {
        if (++v7 >= 400) {
            // all slots occupied — return sentinel slot at end (matches IDA)
            return DAT_07abf5d0 + 366400;
        }
        i += 916;
    }
    FUN_00449840((int)i, (int)i, 0);  // DeleteCloth
    FUN_0045adc0((unsigned char*)i, Type, PosX, PosY, Rotation);
    *(short*)(i + 476) = (short)Key;
    return i;
}

// Wrapper kept for callers that imported the older 3-arg stub signature
// (unused — left here as an inline-compat shim for future ports).
static inline unsigned int CreateChar5(int Key, int Type, int PosX, int PosY)
{
    return FUN_0045bfa0(Key, Type, (unsigned char)PosX, (unsigned char)PosY, 0.0f);
}

// CreateMonster — the big switch.
// Phantom 5th param kept for ABI compat with existing 5-arg call sites.
char* __cdecl FUN_0045ccf0(unsigned int Type_, int PositionX, int PositionY,
                            int Key, int /*phantom_unused*/)
{
    int Type = (int)Type_;
    unsigned int c = 0;
    int v8 = 0;
    int v9, v10;
    int v15;
    unsigned char v16, v17;

    switch (Type) {
    case 1:
    case 5:
        FUN_005098c0(1);
        c = CreateChar5(Key, 271, PositionX, PositionY);
        if (Type == 1) {
            *(unsigned int*)(c + 88) = 0;
            *(unsigned int*)(c + 12) = 1062836634;  // 1.05f
            *(unsigned short*)(c + 624) = 404;
        } else {
            *(unsigned int*)(c + 88) = 1;
            *(unsigned short*)(c + 624) = 407;
            *(unsigned short*)(c + 648) = 601;
            *(unsigned int*)(c + 12) = 1066192077;  // 1.20f
            *(unsigned short*)(c + 446) = 1;
        }
        break;
    case 2:
        FUN_005098c0(2);
        c = CreateChar5(Key, 272, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1056964608;
        break;
    case 3:
        FUN_005098c0(9);
        c = CreateChar5(Key, 279, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1053609165;
        break;
    case 6:
    case 9:
        FUN_005098c0(4);
        c = CreateChar5(Key, 274, PositionX, PositionY);
        if (Type == 6) {
            *(unsigned short*)(c + 624) = 562;
            *(unsigned int*)(c + 12) = 1062836634;
        } else {
            *(unsigned short*)(c + 624) = 563;
            *(unsigned short*)(c + 446) = 1;
            *(unsigned int*)(c + 12) = 1066192077;
        }
        break;
    case 7:
        FUN_005098c0(5);
        c = CreateChar5(Key, 275, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 434;
        *(unsigned short*)(c + 648) = 434;
        *(unsigned int*)(c + 12) = 1070386381;
        break;
    case 10:
        FUN_005098c0(3);
        c = CreateChar5(Key, 273, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1061997773;
        *(unsigned short*)(c + 446) = 1;
        *(unsigned short*)(c + 624) = 413;
        break;
    case 11:
        FUN_005098c0(7);
        c = CreateChar5(Key, 277, PositionX, PositionY);
        *(unsigned int*)(c + 356) = 1053609165;
        *(unsigned short*)(c + 762) = 15;
        *(unsigned char*)(c + 766) = 1;
        break;
    case 12:
        FUN_005098c0(6);
        c = CreateChar5(Key, 276, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1058642330;
        break;
    case 13:
        FUN_005098c0(8);
        c = CreateChar5(Key, 278, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 562;
        *(unsigned int*)(c + 12) = 1066192077;
        break;
    case 14:
    case 55:
    case 56:
        c = CreateChar5(Key, 390, PositionX, PositionY);
        *(unsigned int*)(c + 4) = 206;
        *(unsigned char*)(c + 766) = 1;
        if (Type == 14) {
            *(unsigned int*)(c + 12) = 1064514355;
            *(unsigned short*)(c + 624) = 406;
            *(unsigned short*)(c + 648) = 596;
        } else if (Type == 56) {
            *(unsigned int*)(c + 12) = 1061997773;
            *(unsigned short*)(c + 624) = 504;
        } else {
            *(unsigned short*)(c + 446) = 1;
            *(unsigned int*)(c + 12) = 1068708659;
            *(unsigned short*)(c + 624) = 505;
        }
        break;
    case 15:
        c = CreateChar5(Key, 390, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1066192077;
        *(unsigned short*)(c + 648) = 530;
        *(unsigned int*)(c + 4) = 207;
        *(unsigned short*)(c + 446) = 1;
        *(unsigned char*)(c + 766) = 1;
        break;
    case 16:
        c = CreateChar5(Key, 390, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067030938;
        *(unsigned short*)(c + 624) = 435;
        *(unsigned short*)(c + 648) = 598;
        *(unsigned int*)(c + 4) = 208;
        *(unsigned short*)(c + 446) = 1;
        *(unsigned char*)(c + 766) = 1;
        break;
    case 17:
        FUN_005098c0(10);
        c = CreateChar5(Key, 280, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 440;
        break;
    case 18:
        FUN_005098c0(11);
        c = CreateChar5(Key, 281, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1069547520;
        *(unsigned short*)(c + 624) = 564;
        *(unsigned int*)(c + 100) = 1;
        *(unsigned int*)(c + 104) = 1065353216;  // 1.0f
        break;
    case 19:
        FUN_005098c0(12);
        c = CreateChar5(Key, 282, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1066192077;
        break;
    case 20:
        FUN_005098c0(13);
        c = CreateChar5(Key, 283, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1068708659;
        break;
    case 21:
        FUN_005098c0(14);
        c = CreateChar5(Key, 284, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1064514355;
        break;
    case 22:
        FUN_005098c0(15);
        c = CreateChar5(Key, 285, PositionX, PositionY);
        *(unsigned int*)(c + 100) = 0;
        *(unsigned int*)(c + 104) = 1065353216;
        break;
    case 23:
        FUN_005098c0(16);
        c = CreateChar5(Key, 286, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 439;
        *(unsigned short*)(c + 648) = 602;
        *(unsigned int*)(c + 12) = 1066611507;
        break;
    case 24:
        FUN_005098c0(17);
        c = CreateChar5(Key, 287, PositionX, PositionY);
        break;
    case 25:
        FUN_005098c0(18);
        c = CreateChar5(Key, 288, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 561;
        *(unsigned int*)(c + 100) = 2;
        *(unsigned int*)(c + 104) = 1065353216;
        *(unsigned int*)(c + 12) = 1066192077;
        *(unsigned char*)(c + 220) = 0;
        *(unsigned short*)(c + 446) = 3;
        break;
    case 26:
        FUN_005098c0(19);
        c = CreateChar5(Key, 289, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 432;
        *(unsigned int*)(c + 12) = 1061997773;
        break;
    case 27:
        FUN_005098c0(20);
        c = CreateChar5(Key, 290, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1066192077;
        break;
    case 28:
        FUN_005098c0(21);
        c = CreateChar5(Key, 291, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 497;
        *(unsigned int*)(c + 12) = 1061997773;
        *(unsigned int*)(c + 100) = 1;
        break;
    case 29:
        FUN_005098c0(22);
        c = CreateChar5(Key, 292, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 538;
        *(unsigned int*)(c + 12) = 1064514355;
        break;
    case 30:
        FUN_005098c0(23);
        c = CreateChar5(Key, 293, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1061158912;
        break;
    case 31:
        FUN_005098c0(24);
        c = CreateChar5(Key, 294, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067869798;
        *(unsigned short*)(c + 624) = 408;
        *(unsigned short*)(c + 648) = 408;
        break;
    case 32:
        FUN_005098c0(25);
        c = CreateChar5(Key, 295, PositionX, PositionY);
        break;
    case 33:
        FUN_005098c0(19);
        c = CreateChar5(Key, 289, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 465;
        *(unsigned short*)(c + 648) = 593;
        *(unsigned int*)(c + 12) = 1067030938;
        *(unsigned short*)(c + 446) = 1;
        break;
    case 34:
        c = CreateChar5(Key, 390, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 627;
        *(unsigned short*)(c + 528) = 659;
        *(unsigned short*)(c + 552) = 691;
        *(unsigned short*)(c + 576) = 723;
        *(unsigned short*)(c + 600) = 755;
        *(unsigned short*)(c + 624) = 565;
        *(unsigned short*)(c + 648) = 606;
        *(unsigned char*)(c + 506) = 9;
        *(unsigned char*)(c + 530) = 9;
        *(unsigned char*)(c + 554) = 9;
        *(unsigned char*)(c + 578) = 9;
        *(unsigned char*)(c + 602) = 9;
        *(unsigned char*)(c + 746) = 6;
        SetCharacterScale((int)c);
        if (World == 9) {
            // LABEL_64 path: scale 1067869798
            *(unsigned int*)(c + 12) = 1067869798;
        }
        break;
    case 35:
        FUN_005098c0(11);
        c = CreateChar5(Key, 281, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067869798;
        *(unsigned short*)(c + 624) = 440;
        *(unsigned short*)(c + 648) = 440;
        *(unsigned int*)(c + 100) = 1;
        *(unsigned int*)(c + 104) = 1065353216;
        *(unsigned short*)(c + 446) = 2;
        break;
    case 36:
        FUN_005098c0(28);
        c = CreateChar5(Key, 298, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067030938;
        break;
    case 37:
        FUN_005098c0(26);
        c = CreateChar5(Key, 296, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1066192077;
        break;
    case 38:
    case 67:
        FUN_005098c0(27);
        c = CreateChar5(Key, 297, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 505;
        *(unsigned char*)(c + 626) = 9;
        *(unsigned int*)(c + 12) = 1070386381;
        break;
    case 39:
        FUN_005098c0(28);
        c = CreateChar5(Key, 298, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067030938;
        *(unsigned short*)(c + 446) = 1;
        break;
    case 40:
        FUN_005098c0(29);
        c = CreateChar5(Key, 299, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 414;
        *(unsigned int*)(c + 12) = 1067869798;
        break;
    case 41:
        FUN_005098c0(30);
        c = CreateChar5(Key, 300, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 467;
        *(unsigned int*)(c + 12) = 1066192077;
        break;
    case 42:
        FUN_005098c0(31);
        c = CreateChar5(Key, 301, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067869798;
        *(unsigned int*)(c + 292) = 1128792064;
        *(unsigned int*)(c + 296) = 1125515264;
        *(unsigned int*)(c + 300) = 1133248512;
        break;
    case 43:
        FUN_005098c0(2);
        c = CreateChar5(Key, 272, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1060320051;
        break;
    case 44:
    case 79:
        FUN_005098c0(31);
        c = CreateChar5(Key, 301, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1063675494;
        break;
    case 45:
        FUN_005098c0(33);
        c = CreateChar5(Key, 303, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1058642330;
        break;
    case 46:
    case 81:
        FUN_005098c0(34);
        c = CreateChar5(Key, 304, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1065353216;
        break;
    case 47:
        FUN_005098c0(35);
        c = CreateChar5(Key, 305, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1066192077;
        *(unsigned short*)(c + 624) = 541;
        *(unsigned int*)(c + 100) = 0;
        *(unsigned int*)(c + 104) = 1065353216;
        break;
    case 48:
        FUN_005098c0(36);
        c = CreateChar5(Key, 306, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1068708659;
        *(unsigned short*)(c + 624) = 566;
        break;
    case 49:
        FUN_005098c0(37);
        c = CreateChar5(Key, 307, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned int*)(c + 100) = 5;
        *(unsigned int*)(c + 104) = 0;
        break;
    case 50:
        FUN_005098c0(38);
        c = CreateChar5(Key, 308, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1072064102;
        break;
    case 51:
        FUN_005098c0(33);
        c = CreateChar5(Key, 303, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned short*)(c + 446) = 1;
        break;
    case 52:
        FUN_005098c0(35);
        c = CreateChar5(Key, 305, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1068708659;
        *(unsigned short*)(c + 624) = 541;
        break;
    case 53:
        FUN_005098c0(39);
        c = CreateChar5(Key, 309, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1072064102;
        *(unsigned int*)(c + 100) = 2;
        *(unsigned int*)(c + 104) = 1065353216;
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 54:
    case 151:
        FUN_005098c0(40);
        c = CreateChar5(Key, 310, PositionX, PositionY);
        *(unsigned short*)(c + 648) = 542;
        if (Type == 54) {
            *(unsigned int*)(c + 12) = 1066192077;
        } else {
            *(unsigned int*)(c + 12) = 1067869798;
        }
        break;
    case 57:
        FUN_005098c0(41);
        c = CreateChar5(Key, 311, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1068708659;
        *(unsigned short*)(c + 624) = 542;
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 58:
    case 59:
        FUN_005098c0(42);
        c = CreateChar5(Key, 312, PositionX, PositionY);
        *(unsigned int*)(c + 100) = 2;
        *(unsigned int*)(c + 104) = 1065353216;
        if (Type == 58) {
            *(unsigned int*)(c + 12) = 1072064102;
            *(unsigned short*)(c + 624) = 416;
        } else {
            *(unsigned int*)(c + 12) = 1074161254;
            *(unsigned int*)(c + 4) = 1;
            *(unsigned short*)(c + 624) = 568;
        }
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 60:
        FUN_005098c0(43);
        c = CreateChar5(Key, 313, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1074580685;
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 61:
    case 63:
        FUN_005098c0(44);
        c = CreateChar5(Key, 314, PositionX, PositionY);
        if (Type == 63) {
            *(unsigned int*)(c + 12) = 1072902963;
            *(unsigned int*)(c + 100) = (unsigned int)-2;
            *(unsigned int*)(c + 104) = 1065353216;
        } else {
            *(unsigned int*)(c + 12) = 1069547520;
        }
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 62:
        FUN_005098c0(45);
        c = CreateChar5(Key, 315, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1069547520;
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 64:
        FUN_005098c0(46);
        c = CreateChar5(Key, 316, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067030938;
        *(unsigned short*)(c + 648) = 531;
        *(unsigned char*)(c + 650) = 3;
        break;
    case 65:
        FUN_005098c0(47);
        c = CreateChar5(Key, 317, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067869798;
        break;
    case 66:
        FUN_005098c0(48);
        c = CreateChar5(Key, 318, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1071225242;
        break;
    case 68:
        FUN_005098c0(49);
        c = CreateChar5(Key, 319, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1068708659;
        break;
    case 69:
        FUN_005098c0(50);
        c = CreateChar5(Key, 320, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned int*)(c + 100) = 0;
        break;
    case 70:
        FUN_005098c0(51);
        c = CreateChar5(Key, 321, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1067869798;
        *(unsigned int*)(c + 100) = (unsigned int)-2;
        *(unsigned int*)(c + 104) = 1065353216;
        break;
    case 71:
    case 74:
        FUN_005098c0(52);
        c = CreateChar5(Key, 322, PositionX, PositionY);
        if (Type == 71) {
            *(unsigned int*)(c + 12) = 1066192077;
            *(unsigned short*)(c + 624) = 418;
            *(unsigned char*)(c + 626) = 5;
            *(unsigned short*)(c + 648) = 606;
            *(unsigned char*)(c + 650) = 0;
        } else {
            *(unsigned int*)(c + 12) = 1067869798;
            *(unsigned short*)(c + 624) = 418;
            *(unsigned char*)(c + 626) = 9;
            *(unsigned short*)(c + 648) = 606;
            *(unsigned char*)(c + 650) = 9;
        }
        *(unsigned int*)(c + 100) = 1;
        *(unsigned int*)(c + 104) = 1065353216;
        break;
    case 72:
        FUN_005098c0(53);
        c = CreateChar5(Key, 323, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1069128090;
        *(unsigned short*)(c + 624) = 417;
        *(unsigned char*)(c + 626) = 5;
        break;
    case 73:
    case 75:
        FUN_005098c0(54);
        c = CreateChar5(Key, 324, PositionX, PositionY);
        if (Type == 75) {
            *(unsigned int*)(c + 12) = 1065353216;
        } else {
            *(unsigned int*)(c + 12) = 1061997773;
        }
        // Models[entity_type].Data sub-table flag writes
        {
            unsigned char* modelEntry = (unsigned char*)(uintptr_t)(DAT_05828d58 + 188u * (unsigned)*(short*)(c + 2) + 40u);
            unsigned char* dataPtr = *(unsigned char**)modelEntry;
            if (dataPtr) {
                dataPtr[0]   = 1;
                dataPtr[40]  = 0;
                dataPtr[80]  = 0;
                dataPtr[120] = 1;
                dataPtr[160] = 1;
            }
        }
        break;
    case 77:
        FUN_005098c0(55);
        FUN_005098c0(56);
        c = CreateChar5(Key, 325, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned char*)(uintptr_t)(DAT_05828d58 + 61236u) = 0;
        break;
    case 78:
        FUN_005098c0(19);
        c = CreateChar5(Key, 289, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 432;
        *(unsigned char*)(c + 626) = 9;
        *(unsigned int*)(c + 12) = 1061997773;
        break;
    case 80:
        FUN_005098c0(36);
        c = CreateChar5(Key, 306, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1068708659;
        *(unsigned short*)(c + 624) = 567;
        *(unsigned char*)(c + 627) = 63;
        break;
    case 82:
        FUN_005098c0(42);
        c = CreateChar5(Key, 312, PositionX, PositionY);
        *(unsigned int*)(c + 100) = 2;
        *(unsigned int*)(c + 104) = 1065353216;
        *(unsigned int*)(c + 12) = 1072064102;
        *(unsigned short*)(c + 624) = 416;
        *(unsigned char*)(c + 627) = 63;
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 83:
        FUN_005098c0(41);
        c = CreateChar5(Key, 311, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1068708659;
        *(unsigned short*)(c + 624) = 542;
        *(unsigned char*)(c + 627) = 63;
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
        Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
        break;
    case 84: case 90: case 96: case 113: case 119: case 125:
        FUN_005098c0(47);
        c = CreateChar5(Key, 317, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1066192077;
        break;
    case 85: case 91: case 97: case 114: case 120: case 126:
        FUN_005098c0(46);
        c = CreateChar5(Key, 316, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1066192077;
        *(unsigned short*)(c + 648) = 531;
        *(unsigned char*)(c + 650) = 1;
        break;
    case 86: case 92: case 98: case 115: case 121: case 127:
        FUN_005098c0(59);
        c = CreateChar5(Key, 329, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 440;
        *(unsigned char*)(c + 626) = 0;
        *(unsigned short*)(c + 648) = 440;
        *(unsigned char*)(c + 650) = 0;
        *(unsigned int*)(c + 12) = 1065353216;
        break;
    case 87: case 93: case 99: case 116: case 122: case 128:
        FUN_005098c0(58);
        c = CreateChar5(Key, 328, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1061997773;
        break;
    case 88: case 94: case 111: case 117: case 123: case 129:
        FUN_005098c0(57);
        c = CreateChar5(Key, 327, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 470;
        if ((World - 9) / 3) {
            *(unsigned char*)(c + 626) = 0;
        } else {
            *(unsigned char*)(c + 626) = 8;
        }
        *(unsigned int*)(c + 12) = 1066947052;
        break;
    case 89: case 95: case 112: case 118: case 124: case 130:
        FUN_005098c0(62);
        c = CreateChar5(Key, 332, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 560;
        *(unsigned char*)(c + 626) = 11;
        *(unsigned int*)(c + 12) = 1067030938;
        break;
    case 100:
        c = CreateChar5(Key, 39, PositionX, PositionY);
        break;
    case 101:
        c = CreateChar5(Key, 40, PositionX, PositionY);
        break;
    case 102:
        c = CreateChar5(Key, 51, PositionX, PositionY);
        break;
    case 103:
        c = CreateChar5(Key, 25, PositionX, PositionY);
        break;
    case 131:
        FUN_005098c0(61);
        c = CreateChar5(Key, 331, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1061997773;
        *(unsigned char*)(c + 140) = 0;
        break;
    case 132: case 133: case 134:
        FUN_005098c0(60);
        c = CreateChar5(Key, 330, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1061997773;
        *(unsigned char*)(c + 140) = 0;
        break;
    case 150:
        FUN_005098c0(32);
        c = CreateChar5(Key, 302, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1039516303;
        break;
    case 200:
        c = CreateChar5(Key, 236, PositionX, PositionY);
        *(unsigned int*)(c + 100) = 2;
        *(unsigned int*)(c + 12) = 1072064102;
        *(unsigned short*)(c + 446) = 1;
        break;
    case 230:
        OpenNpc_stub(336);
        c = CreateChar5(Key, 336, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 360;
        *(unsigned short*)(c + 528) = 363;
        *(unsigned short*)(c + 576) = 365;
        *(unsigned short*)(c + 600) = 366;
        break;
    case 231:
        OpenNpc_stub(377);
        c = CreateChar5(Key, 377, PositionX, PositionY);
        break;
    case 232:
        OpenNpc_stub(375);
        v17 = (unsigned char)PositionY;
        v16 = (unsigned char)PositionX;
        v15 = 375;
        c = FUN_0045bfa0(Key, v15, v16, v17, 0.0f);
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned char*)(c + 132) = 4;
        break;
    case 233:
        OpenNpc_stub(376);
        v17 = (unsigned char)PositionY;
        v16 = (unsigned char)PositionX;
        v15 = 376;
        c = FUN_0045bfa0(Key, v15, v16, v17, 0.0f);
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned char*)(c + 132) = 4;
        break;
    case 234:
        FUN_005098c0(19);
        c = CreateChar5(Key, 289, PositionX, PositionY);
        *(unsigned short*)(c + 624) = 560;
        *(unsigned char*)(c + 626) = 4;
        *(unsigned int*)(c + 12) = 1069547520;
        *(unsigned char*)(c + 132) = 4;
        FUN_0043e820((int)c, 0);
        break;
    case 235:
        OpenNpc_stub(374);
        v17 = (unsigned char)PositionY;
        v16 = (unsigned char)PositionX;
        v15 = 374;
        c = FUN_0045bfa0(Key, v15, v16, v17, 0.0f);
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned char*)(c + 132) = 4;
        break;
    case 236:
        OpenNpc_stub(390);
        c = CreateChar5(Key, 390, PositionX, PositionY);
        *(unsigned int*)(c + 4) = 207;
        *(unsigned int*)(c + 12) = 1065353216;
        *(unsigned char*)(c + 132) = 4;
        *(unsigned short*)(c + 446) = 8;
        break;
    case 237:
        OpenNpc_stub(349);
        c = CreateChar5(Key, 349, PositionX, PositionY);
        break;
    case 238:
        OpenNpc_stub(348);
        c = CreateChar5(Key, 348, PositionX, PositionY);
        *(unsigned int*)(c + 100) = 1;
        break;
    case 239:
        OpenNpc_stub(347);
        c = CreateChar5(Key, 347, PositionX, PositionY);
        break;
    case 240:
        OpenNpc_stub(346);
        c = CreateChar5(Key, 346, PositionX, PositionY);
        break;
    case 241:
        OpenNpc_stub(345);
        c = CreateChar5(Key, 345, PositionX, PositionY);
        break;
    case 242: {
        OpenNpc_stub(343);
        c = CreateChar5(Key, 343, PositionX, PositionY);
        float v13 = *(float*)(c + 16);
        float v14 = *(float*)(c + 20);
        *(unsigned int*)(c + 100) = 1;
        *(float*)(c + 24) = FUN_004f7500(v13, v14) + 140.0f;
        break;
    }
    case 243:
        OpenNpc_stub(344);
        c = CreateChar5(Key, 344, PositionX, PositionY);
        break;
    case 244:
        OpenNpc_stub(340);
        c = CreateChar5(Key, 340, PositionX, PositionY);
        break;
    case 245:
        OpenNpc_stub(342);
        c = CreateChar5(Key, 342, PositionX, PositionY);
        break;
    case 246:
        OpenNpc_stub(341);
        c = CreateChar5(Key, 341, PositionX, PositionY);
        break;
    case 247:
        c = CreateChar5(Key, 390, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 633;
        *(unsigned short*)(c + 528) = 665;
        *(unsigned short*)(c + 552) = 697;
        *(unsigned short*)(c + 576) = 729;
        *(unsigned short*)(c + 600) = 761;
        *(unsigned short*)(c + 624) = 539;
        *(unsigned short*)(c + 648) = 535;
        SetCharacterScale((int)c);
        break;
    case 248:
        OpenNpc_stub(336);
        c = CreateChar5(Key, 336, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 361;
        *(unsigned short*)(c + 528) = 363;
        *(unsigned short*)(c + 576) = 365;
        *(unsigned short*)(c + 600) = 367;
        break;
    case 249:
        c = CreateChar5(Key, 390, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 633;
        *(unsigned short*)(c + 528) = 665;
        *(unsigned short*)(c + 552) = 697;
        *(unsigned short*)(c + 576) = 729;
        *(unsigned short*)(c + 600) = 761;
        *(unsigned short*)(c + 624) = 503;
        SetCharacterScale((int)c);
        break;
    case 250:
        OpenNpc_stub(336);
        c = CreateChar5(Key, 336, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 360;
        *(unsigned short*)(c + 528) = 362;
        *(unsigned short*)(c + 576) = 364;
        *(unsigned short*)(c + 600) = 366;
        break;
    case 251:
        OpenNpc_stub(338);
        c = CreateChar5(Key, 338, PositionX, PositionY);
        *(unsigned int*)(c + 12) = 1064514355;
        break;
    case 253:
        OpenNpc_stub(337);
        c = CreateChar5(Key, 337, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 368;
        *(unsigned short*)(c + 528) = 370;
        *(unsigned short*)(c + 552) = 372;
        break;
    case 254:
        OpenNpc_stub(339);
        c = CreateChar5(Key, 339, PositionX, PositionY);
        break;
    case 255:
        OpenNpc_stub(335);
        c = CreateChar5(Key, 335, PositionX, PositionY);
        *(unsigned short*)(c + 504) = 351;
        *(unsigned short*)(c + 528) = 353;
        *(unsigned short*)(c + 552) = 355;
        *(unsigned short*)(c + 600) = 359;
        break;
    default:
        FUN_005098c0(0);
        c = CreateChar5(Key, 270, PositionX, PositionY);
        if (Type) {
            if (Type == 4) {
                *(unsigned short*)(c + 624) = 503;
                *(unsigned int*)(c + 12) = 1066611507;
                *(unsigned short*)(c + 446) = 1;
            } else if (Type == 8) {
                *(unsigned short*)(c + 624) = 504;
                *(unsigned int*)(c + 12) = 1065353216;
                *(unsigned short*)(c + 446) = 2;
                *(unsigned int*)(c + 120) = 1;
            }
        } else {
            *(unsigned int*)(c + 88) = 0;
            *(unsigned int*)(c + 12) = 1061997773;
            *(unsigned short*)(c + 624) = 438;
        }
        break;
    }

    // World-tier scale bump for monsters 84..136 in worlds 9..16
    v9 = World - 9;
    if (v9 > 0 && v9 <= 7) {
        v10 = Type;
        if (Type >= 84 && Type <= 136) {
            *(unsigned short*)(c + 446) = 0;
            *(float*)(c + 12) = (float)((double)(v9 / 3) * 0.05000000074505806 + *(float*)(c + 12));
        }
    } else {
        v10 = Type;
    }

    if (c) {
        // RE-ACTIVADO 2026-07-24: copiar el nombre desde la tabla MonsterScript/
        // NPCName (getMonsterName por Type).  Antes se salteaba porque la tabla
        // era un global de 1 byte; ahora esta bien dimensionada y cargada desde
        // NPCName.txt.  El nombre va a c+0x1c1 (449) — lo lee Target_Render al
        // hacer hover sobre el NPC/mob.  Usa el Type ORIGINAL del packet (el
        // blacksmith es 251), no el model type resuelto.
        {
            char* mname = getMonsterName(Type);
            char* dst = (char*)(uintptr_t)(c + 0x1c1);
            if (mname && mname[0]) {
                int n = 0;
                while (mname[n] != '\0' && n < 31) { dst[n] = mname[n]; n++; }
                dst[n] = '\0';
            } else {
                dst[0] = '\0';
            }
        }
        v8 = 0;  // suppress unused-warning

        *(unsigned char*)(c + 747) = (unsigned char)v10;
        *(unsigned short*)(c + 8) = 0;  // v18 was uninit stack — defaults to 0
        *(unsigned short*)(c + 784) = (unsigned short)HeroIndex;

        if (v10 == 200) {
            *(unsigned char*)(c + 132) = 2;
        } else if (v10 > 200) {
            *(unsigned char*)(c + 132) = 4;
            return (char*)(uintptr_t)c;
        } else if (v10 < 150 && v10 <= 110 && v10 >= 100) {
            *(unsigned char*)(c + 132) = 8;
            return (char*)(uintptr_t)c;
        } else {
            *(unsigned char*)(c + 132) = 2;
        }
    }
    return (char*)(uintptr_t)c;
}
