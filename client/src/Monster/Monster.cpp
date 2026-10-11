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
//         Widget_Release(entity[+0x184][i * 0x15])  // release sub-mesh i
//       release(entity[+0x184])            // release pointer array
//       entity[+0x184] = NULL
//       entity[+0x180] = 0
//
//     if (param1 != 0):
//       ptr = entity + 500                  // offset 0x1F4
//       for i in 0..5:                      // 6 attach points
//         if ptr[i*6] != NULL:
//           Widget_Release(ptr[i*6])           // release attach mesh
//           ptr[i*6] = NULL
//
//     if (param3 != 0 && entity[+0x14] != NULL):
//       Widget_Release(entity[+0x14])          // release extra mesh
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
//     // Configura sound samples por tipo (LoadWaveFile):
//     LoadWaveFile(sound_id, &sound_data, 2, 1) — 5 sonidos por monstruo:
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
//       BMD__Save(entity_def, folder, path_bmd)  // BMD_LoadFile
//     else:
//       BMD__Open(entity_def, folder, path_bmd)  // BMD_LoadCompressed
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
//         PlayBuffer(0x68, entity, 0)          // UI event 0x68
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
//         PlayBuffer(0x67, 0, 0)               // UI event 0x67
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
//     0x14 = spawn individual (Entity_FindById + DeleteCharacter)
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
//   PacketHandler_0x14      @ Net_Process opcode 0x14 (DeleteCharacter)
//   Particle_Spawn          @ 0x00460dc0  (ver Combat.cpp)
//   Particle_StartLoop      @ 0x0046fe00  (ver Particle_Render.cpp)
//   Particle_StopLoop       @ 0x00460d20

#include "stdafx.h"
#include <array>
#include "Item/ContentCatalog.h"
#include "Monster/Monster.h"

// IDA: CreateMonster (0x0045CCF0).
// La definición vanilla carga el modelo y configura escala, equipo y flags.
// Los casos con joints, altura o escala calculada usan inicializadores específicos.
// El catálogo conserva prioridad de modelo/escala; getMonsterName resuelve el nombre.
// Se mantienen el ajuste por mundo y las marcas finales de tipo, clase de entidad y HeroIndex.
extern "C++" {
extern void __cdecl OpenNpc(int Type);
}

// IDA: CreateCharacter (0x0045BFA0)
// CreateCharacter — finds/allocates an entity slot for Key, returns pointer.
// 1) scan first 400 slots for matching key at +476 → reuse slot
// 2) else find first inactive slot (active flag at +0 == 0) → init it
// Returns pointer (DWORD) into CharactersClient (DAT_07abf5d0).
unsigned int __cdecl CreateCharacter(int Key, int Type, unsigned char PosX,
                                   unsigned char PosY, float Rotation)
{
    unsigned int c = DAT_07abf5d0;
    for (int i = 0; i < 400; ++i) {
        if (*(unsigned char*)c != 0 && *(short*)(c + 476) == (short)Key) {
            CreateCharacterPointer((unsigned char*)c, Type, PosX, PosY, Rotation);
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
    DeleteCloth((int)i, (int)i, 0);  // DeleteCloth
    CreateCharacterPointer((unsigned char*)i, Type, PosX, PosY, Rotation);
    *(short*)(i + 476) = (short)Key;
    return i;
}

// Wrapper kept for callers that imported the older 3-arg stub signature
// (unused — left here as an inline-compat shim for future ports).
static inline unsigned int CreateChar5(int Key, int Type, int PosX, int PosY)
{
    return CreateCharacter(Key, Type, (unsigned char)PosX, (unsigned char)PosY, 0.0f);
}


namespace {
// IDA: CreateMonster (0x0045CCF0). Los campos ausentes conservan lo que dejó CreateCharacter.
constexpr int KeepSpawnValue = -32768;
using SpawnInitializer = void (*)(unsigned int);
enum class SpawnLoader { None, Monster, Npc };

struct MonsterDefinition {
    int Model = 270;
    SpawnLoader Loader = SpawnLoader::Monster;
    int Resource = 0;
    int ExtraResource = -1;
    float Scale = 0.0f;
    int Right = -1, Left = -1;
    int RightLevel = -1, LeftLevel = -1, RightExcellent = -1;
    int BlendMesh = KeepSpawnValue;
    float BlendLight = -1.0f;
    int Subtype = -1, HiddenMesh = KeepSpawnValue;
    int State446 = -1, Flag766 = -1;
    SpawnInitializer Initialize = nullptr;

    constexpr MonsterDefinition() = default;
    constexpr MonsterDefinition(int model, SpawnLoader loader, int resource = 0)
        : Model(model), Loader(loader), Resource(resource) {}
    constexpr MonsterDefinition& WithScale(float value) { Scale = value; return *this; }
    constexpr MonsterDefinition& WithRight(int model, int level = -1, int excellent = -1) {
        Right = model; RightLevel = level; RightExcellent = excellent; return *this;
    }
    constexpr MonsterDefinition& WithLeft(int model, int level = -1) {
        Left = model; LeftLevel = level; return *this;
    }
    constexpr MonsterDefinition& WithBlend(int mesh, float light = -1.0f) {
        BlendMesh = mesh; BlendLight = light; return *this;
    }
    constexpr MonsterDefinition& WithSubtype(int value) { Subtype = value; return *this; }
    constexpr MonsterDefinition& WithHiddenMesh(int value) { HiddenMesh = value; return *this; }
    constexpr MonsterDefinition& WithState446(int value) { State446 = value; return *this; }
    constexpr MonsterDefinition& WithFlag766(int value) { Flag766 = value; return *this; }
    constexpr MonsterDefinition& WithExtraResource(int value) { ExtraResource = value; return *this; }
    constexpr MonsterDefinition& WithInitializer(SpawnInitializer value) { Initialize = value; return *this; }
};

// IDA: CreateMonster (0x0045CCF0), monstruos que crean el par de joints 1258.
void InitializeMonsterJoints(unsigned int c)
{
    Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (int)c, 30.0f, -1, 0);
    Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (int)c, 30.0f, -1, 0);
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 8.
void InitializeMonster8(unsigned int c)
{
    *(unsigned int*)(c + 120) = 1;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 11.
void InitializeMonster11(unsigned int c)
{
    *(unsigned int*)(c + 356) = 1053609165;
    *(unsigned short*)(c + 762) = 15;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 25.
void InitializeMonster25(unsigned int c)
{
    *(unsigned char*)(c + 220) = 0;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 34.
void InitializeMonster34(unsigned int c)
{
    *(unsigned short*)(c + 504) = MODEL_HELM + 3;
    *(unsigned short*)(c + 528) = MODEL_ARMOR + 3;
    *(unsigned short*)(c + 552) = MODEL_PANTS + 3;
    *(unsigned short*)(c + 576) = MODEL_GLOVES + 3;
    *(unsigned short*)(c + 600) = MODEL_BOOTS + 3;
    *(unsigned char*)(c + 506) = 9;
    *(unsigned char*)(c + 530) = 9;
    *(unsigned char*)(c + 554) = 9;
    *(unsigned char*)(c + 578) = 9;
    *(unsigned char*)(c + 602) = 9;
    *(unsigned char*)(c + 746) = 6;
    SetCharacterScale((int)c);
    if (World == 9) {
        *(unsigned int*)(c + 12) = 1067869798;
    }
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 42.
void InitializeMonster42(unsigned int c)
{
    *(unsigned int*)(c + 292) = 1128792064;
    *(unsigned int*)(c + 296) = 1125515264;
    *(unsigned int*)(c + 300) = 1133248512;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 73.
void InitializeMonster73(unsigned int c)
{
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
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 77.
void InitializeMonster77(unsigned int c)
{
    *(unsigned char*)(uintptr_t)(DAT_05828d58 + 61236u) = 0;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 88.
void InitializeMonster88(unsigned int c)
{
    if ((World - 9) / 3) {
        *(unsigned char*)(c + 626) = 0;
    } else {
        *(unsigned char*)(c + 626) = 8;
    }
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 131.
void InitializeMonster131(unsigned int c)
{
    *(unsigned char*)(c + 140) = 0;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 230.
void InitializeMonster230(unsigned int c)
{
    *(unsigned short*)(c + 504) = 360;
    *(unsigned short*)(c + 528) = 363;
    *(unsigned short*)(c + 576) = 365;
    *(unsigned short*)(c + 600) = 366;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 232.
void InitializeMonster232(unsigned int c)
{
    *(unsigned char*)(c + 132) = 4;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 234.
void InitializeMonster234(unsigned int c)
{
    *(unsigned char*)(c + 132) = 4;
    SetAction((int)c, 0);
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 242.
void InitializeMonster242(unsigned int c)
{
    float v13 = *(float*)(c + 16);
    float v14 = *(float*)(c + 20);
    *(float*)(c + 24) = RequestTerrainHeight(v13, v14) + 140.0f;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 247.
void InitializeMonster247(unsigned int c)
{
    *(unsigned short*)(c + 504) = MODEL_HELM + 9;
    *(unsigned short*)(c + 528) = MODEL_ARMOR + 9;
    *(unsigned short*)(c + 552) = MODEL_PANTS + 9;
    *(unsigned short*)(c + 576) = MODEL_GLOVES + 9;
    *(unsigned short*)(c + 600) = MODEL_BOOTS + 9;
    SetCharacterScale((int)c);
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 248.
void InitializeMonster248(unsigned int c)
{
    *(unsigned short*)(c + 504) = 361;
    *(unsigned short*)(c + 528) = 363;
    *(unsigned short*)(c + 576) = 365;
    *(unsigned short*)(c + 600) = 367;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 250.
void InitializeMonster250(unsigned int c)
{
    *(unsigned short*)(c + 504) = 360;
    *(unsigned short*)(c + 528) = 362;
    *(unsigned short*)(c + 576) = 364;
    *(unsigned short*)(c + 600) = 366;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 253.
void InitializeMonster253(unsigned int c)
{
    *(unsigned short*)(c + 504) = 368;
    *(unsigned short*)(c + 528) = 370;
    *(unsigned short*)(c + 552) = 372;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 255.
void InitializeMonster255(unsigned int c)
{
    *(unsigned short*)(c + 504) = 351;
    *(unsigned short*)(c + 528) = 353;
    *(unsigned short*)(c + 552) = 355;
    *(unsigned short*)(c + 600) = 359;
}

// Tabla indexada por tipo vanilla; los huecos y los tipos extendidos sin modelo propio
// conservan el caso default. El nombre sigue saliendo de getMonsterName/MonsterScript,
// sin reintroducir los strings fijos omitidos por el port. Los inicializadores se ejecutan después de los datos comunes.
constexpr std::array<MonsterDefinition, 256> MakeMonsterDefinitions()
{
    std::array<MonsterDefinition, 256> definitions{};
    definitions[0] = MonsterDefinition(270, SpawnLoader::Monster, 0).WithScale(0.8f).WithRight(MODEL_AXE + 6).WithHiddenMesh(0);
    definitions[1] = MonsterDefinition(271, SpawnLoader::Monster, 1).WithScale(0.85f).WithRight(MODEL_SWORD + 4).WithHiddenMesh(0);
    definitions[2] = MonsterDefinition(272, SpawnLoader::Monster, 2).WithScale(0.5f);
    definitions[3] = MonsterDefinition(279, SpawnLoader::Monster, 9).WithScale(0.4f);
    definitions[4] = MonsterDefinition(270, SpawnLoader::Monster, 0).WithScale(1.15f).WithRight(MODEL_SPEAR + 7).WithState446(1);
    definitions[5] = MonsterDefinition(271, SpawnLoader::Monster, 1)
        .WithScale(1.1f)
        .WithRight(MODEL_SWORD + 7)
        .WithLeft(MODEL_SHIELD + 9)
        .WithHiddenMesh(1)
        .WithState446(1);
    definitions[6] = MonsterDefinition(274, SpawnLoader::Monster, 4).WithScale(0.85f).WithRight(MODEL_STAFF + 2);
    definitions[7] = MonsterDefinition(275, SpawnLoader::Monster, 5).WithScale(1.6f).WithRight(MODEL_AXE + 2).WithLeft(MODEL_AXE + 2);
    definitions[8] = MonsterDefinition(270, SpawnLoader::Monster, 0)
        .WithScale(1.0f)
        .WithRight(MODEL_SPEAR + 8)
        .WithState446(2)
        .WithInitializer(InitializeMonster8);
    definitions[9] = MonsterDefinition(274, SpawnLoader::Monster, 4).WithScale(1.1f).WithRight(MODEL_STAFF + 3).WithState446(1);
    definitions[10] = MonsterDefinition(273, SpawnLoader::Monster, 3).WithScale(0.8f).WithRight(MODEL_SWORD + 13).WithState446(1);
    definitions[11] = MonsterDefinition(277, SpawnLoader::Monster, 7).WithFlag766(1).WithInitializer(InitializeMonster11);
    definitions[12] = MonsterDefinition(276, SpawnLoader::Monster, 6).WithScale(0.6f);
    definitions[13] = MonsterDefinition(278, SpawnLoader::Monster, 8).WithScale(1.1f).WithRight(MODEL_STAFF + 2);
    definitions[14] = MonsterDefinition(390, SpawnLoader::None)
        .WithScale(0.95f)
        .WithRight(MODEL_SWORD + 6)
        .WithLeft(MODEL_SHIELD + 4)
        .WithSubtype(206)
        .WithFlag766(1);
    definitions[15] = MonsterDefinition(390, SpawnLoader::None)
        .WithScale(1.1f)
        .WithLeft(MODEL_BOW + 2)
        .WithSubtype(207)
        .WithState446(1)
        .WithFlag766(1);
    definitions[16] = MonsterDefinition(390, SpawnLoader::None)
        .WithScale(1.2f)
        .WithRight(MODEL_AXE + 3)
        .WithLeft(MODEL_SHIELD + 6)
        .WithSubtype(208)
        .WithState446(1)
        .WithFlag766(1);
    definitions[17] = MonsterDefinition(280, SpawnLoader::Monster, 10).WithRight(MODEL_AXE + 8);
    definitions[18] = MonsterDefinition(281, SpawnLoader::Monster, 11).WithScale(1.5f).WithRight(MODEL_STAFF + 4).WithBlend(1, 1.0f);
    definitions[19] = MonsterDefinition(282, SpawnLoader::Monster, 12).WithScale(1.1f);
    definitions[20] = MonsterDefinition(283, SpawnLoader::Monster, 13).WithScale(1.4f);
    definitions[21] = MonsterDefinition(284, SpawnLoader::Monster, 14).WithScale(0.95f);
    definitions[22] = MonsterDefinition(285, SpawnLoader::Monster, 15).WithBlend(0, 1.0f);
    definitions[23] = MonsterDefinition(286, SpawnLoader::Monster, 16).WithScale(1.15f).WithRight(MODEL_AXE + 7).WithLeft(MODEL_SHIELD + 10);
    definitions[24] = MonsterDefinition(287, SpawnLoader::Monster, 17);
    definitions[25] = MonsterDefinition(288, SpawnLoader::Monster, 18)
        .WithScale(1.1f)
        .WithRight(MODEL_STAFF + 1)
        .WithBlend(2, 1.0f)
        .WithState446(3)
        .WithInitializer(InitializeMonster25);
    definitions[26] = MonsterDefinition(289, SpawnLoader::Monster, 19).WithScale(0.8f).WithRight(MODEL_AXE);
    definitions[27] = MonsterDefinition(290, SpawnLoader::Monster, 20).WithScale(1.1f);
    definitions[28] = MonsterDefinition(291, SpawnLoader::Monster, 21).WithScale(0.8f).WithRight(MODEL_SPEAR + 1).WithBlend(1);
    definitions[29] = MonsterDefinition(292, SpawnLoader::Monster, 22).WithScale(0.95f).WithRight(MODEL_BOW + 10);
    definitions[30] = MonsterDefinition(293, SpawnLoader::Monster, 23).WithScale(0.75f);
    definitions[31] = MonsterDefinition(294, SpawnLoader::Monster, 24).WithScale(1.3f).WithRight(MODEL_SWORD + 8).WithLeft(MODEL_SWORD + 8);
    definitions[32] = MonsterDefinition(295, SpawnLoader::Monster, 25);
    definitions[33] = MonsterDefinition(289, SpawnLoader::Monster, 19)
        .WithScale(1.2f)
        .WithRight(MODEL_MACE + 1)
        .WithLeft(MODEL_SHIELD + 1)
        .WithState446(1);
    definitions[34] = MonsterDefinition(390, SpawnLoader::None)
        .WithRight(MODEL_STAFF + 5)
        .WithLeft(MODEL_SHIELD + 14)
        .WithInitializer(InitializeMonster34);
    definitions[35] = MonsterDefinition(281, SpawnLoader::Monster, 11)
        .WithScale(1.3f)
        .WithRight(MODEL_AXE + 8)
        .WithLeft(MODEL_AXE + 8)
        .WithBlend(1, 1.0f)
        .WithState446(2);
    definitions[36] = MonsterDefinition(298, SpawnLoader::Monster, 28).WithScale(1.2f);
    definitions[37] = MonsterDefinition(296, SpawnLoader::Monster, 26).WithScale(1.1f);
    definitions[38] = MonsterDefinition(297, SpawnLoader::Monster, 27).WithScale(1.6f).WithRight(MODEL_SPEAR + 9, 9);
    definitions[39] = MonsterDefinition(298, SpawnLoader::Monster, 28).WithScale(1.2f).WithState446(1);
    definitions[40] = MonsterDefinition(299, SpawnLoader::Monster, 29).WithScale(1.3f).WithRight(MODEL_SWORD + 14);
    definitions[41] = MonsterDefinition(300, SpawnLoader::Monster, 30).WithScale(1.1f).WithRight(MODEL_MACE + 3);
    definitions[42] = MonsterDefinition(301, SpawnLoader::Monster, 31).WithScale(1.3f).WithInitializer(InitializeMonster42);
    definitions[43] = MonsterDefinition(272, SpawnLoader::Monster, 2).WithScale(0.7f);
    definitions[44] = MonsterDefinition(301, SpawnLoader::Monster, 31).WithScale(0.9f);
    definitions[45] = MonsterDefinition(303, SpawnLoader::Monster, 33).WithScale(0.6f);
    definitions[46] = MonsterDefinition(304, SpawnLoader::Monster, 34).WithScale(1.0f);
    definitions[47] = MonsterDefinition(305, SpawnLoader::Monster, 35).WithScale(1.1f).WithRight(MODEL_BOW + 13).WithBlend(0, 1.0f);
    definitions[48] = MonsterDefinition(306, SpawnLoader::Monster, 36).WithScale(1.4f).WithRight(MODEL_STAFF + 6);
    definitions[49] = MonsterDefinition(307, SpawnLoader::Monster, 37).WithScale(1.0f).WithBlend(5, 0.0f);
    definitions[50] = MonsterDefinition(308, SpawnLoader::Monster, 38).WithScale(1.8f);
    definitions[51] = MonsterDefinition(303, SpawnLoader::Monster, 33).WithScale(1.0f).WithState446(1);
    definitions[52] = MonsterDefinition(305, SpawnLoader::Monster, 35).WithScale(1.4f).WithRight(MODEL_BOW + 13);
    definitions[53] = MonsterDefinition(309, SpawnLoader::Monster, 39)
        .WithScale(1.8f)
        .WithBlend(2, 1.0f)
        .WithInitializer(InitializeMonsterJoints);
    definitions[54] = MonsterDefinition(310, SpawnLoader::Monster, 40).WithScale(1.1f).WithLeft(MODEL_BOW + 14);
    definitions[55] = MonsterDefinition(390, SpawnLoader::None)
        .WithScale(1.4f)
        .WithRight(MODEL_SPEAR + 9)
        .WithSubtype(206)
        .WithState446(1)
        .WithFlag766(1);
    definitions[56] = MonsterDefinition(390, SpawnLoader::None).WithScale(0.8f).WithRight(MODEL_SPEAR + 8).WithSubtype(206).WithFlag766(1);
    definitions[57] = MonsterDefinition(311, SpawnLoader::Monster, 41)
        .WithScale(1.4f)
        .WithRight(MODEL_BOW + 14)
        .WithInitializer(InitializeMonsterJoints);
    definitions[58] = MonsterDefinition(312, SpawnLoader::Monster, 42)
        .WithScale(1.8f)
        .WithRight(MODEL_SWORD + 16)
        .WithBlend(2, 1.0f)
        .WithInitializer(InitializeMonsterJoints);
    definitions[59] = MonsterDefinition(312, SpawnLoader::Monster, 42)
        .WithScale(2.1f)
        .WithRight(MODEL_STAFF + 8)
        .WithBlend(2, 1.0f)
        .WithSubtype(1)
        .WithInitializer(InitializeMonsterJoints);
    definitions[60] = MonsterDefinition(313, SpawnLoader::Monster, 43).WithScale(2.2f).WithInitializer(InitializeMonsterJoints);
    definitions[61] = MonsterDefinition(314, SpawnLoader::Monster, 44).WithScale(1.5f).WithInitializer(InitializeMonsterJoints);
    definitions[62] = MonsterDefinition(315, SpawnLoader::Monster, 45).WithScale(1.5f).WithInitializer(InitializeMonsterJoints);
    definitions[63] = MonsterDefinition(314, SpawnLoader::Monster, 44)
        .WithScale(1.9f)
        .WithBlend(-2, 1.0f)
        .WithInitializer(InitializeMonsterJoints);
    definitions[64] = MonsterDefinition(316, SpawnLoader::Monster, 46).WithScale(1.2f).WithLeft(MODEL_BOW + 3, 3);
    definitions[65] = MonsterDefinition(317, SpawnLoader::Monster, 47).WithScale(1.3f);
    definitions[66] = MonsterDefinition(318, SpawnLoader::Monster, 48).WithScale(1.7f);
    definitions[67] = MonsterDefinition(297, SpawnLoader::Monster, 27).WithScale(1.6f).WithRight(MODEL_SPEAR + 9, 9);
    definitions[68] = MonsterDefinition(319, SpawnLoader::Monster, 49).WithScale(1.4f);
    definitions[69] = MonsterDefinition(320, SpawnLoader::Monster, 50).WithScale(1.0f).WithBlend(0);
    definitions[70] = MonsterDefinition(321, SpawnLoader::Monster, 51).WithScale(1.3f).WithBlend(-2, 1.0f);
    definitions[71] = MonsterDefinition(322, SpawnLoader::Monster, 52)
        .WithScale(1.1f)
        .WithRight(MODEL_SWORD + 18, 5)
        .WithLeft(MODEL_SHIELD + 14, 0)
        .WithBlend(1, 1.0f);
    definitions[72] = MonsterDefinition(323, SpawnLoader::Monster, 53).WithScale(1.45f).WithRight(MODEL_SWORD + 17, 5);
    definitions[73] = MonsterDefinition(324, SpawnLoader::Monster, 54).WithScale(0.8f).WithInitializer(InitializeMonster73);
    definitions[74] = MonsterDefinition(322, SpawnLoader::Monster, 52)
        .WithScale(1.3f)
        .WithRight(MODEL_SWORD + 18, 9)
        .WithLeft(MODEL_SHIELD + 14, 9)
        .WithBlend(1, 1.0f);
    definitions[75] = MonsterDefinition(324, SpawnLoader::Monster, 54).WithScale(1.0f).WithInitializer(InitializeMonster73);
    definitions[77] = MonsterDefinition(325, SpawnLoader::Monster, 55)
        .WithExtraResource(56)
        .WithScale(1.0f)
        .WithInitializer(InitializeMonster77);
    definitions[78] = MonsterDefinition(289, SpawnLoader::Monster, 19).WithScale(0.8f).WithRight(MODEL_AXE, 9);
    definitions[79] = MonsterDefinition(301, SpawnLoader::Monster, 31).WithScale(0.9f);
    definitions[80] = MonsterDefinition(306, SpawnLoader::Monster, 36).WithScale(1.4f).WithRight(MODEL_STAFF + 7, -1, 63);
    definitions[81] = MonsterDefinition(304, SpawnLoader::Monster, 34).WithScale(1.0f);
    definitions[82] = MonsterDefinition(312, SpawnLoader::Monster, 42)
        .WithScale(1.8f)
        .WithRight(MODEL_SWORD + 16, -1, 63)
        .WithBlend(2, 1.0f)
        .WithInitializer(InitializeMonsterJoints);
    definitions[83] = MonsterDefinition(311, SpawnLoader::Monster, 41)
        .WithScale(1.4f)
        .WithRight(MODEL_BOW + 14, -1, 63)
        .WithInitializer(InitializeMonsterJoints);
    definitions[84] = MonsterDefinition(317, SpawnLoader::Monster, 47).WithScale(1.1f);
    definitions[85] = MonsterDefinition(316, SpawnLoader::Monster, 46).WithScale(1.1f).WithLeft(MODEL_BOW + 3, 1);
    definitions[86] = MonsterDefinition(329, SpawnLoader::Monster, 59).WithScale(1.0f).WithRight(MODEL_AXE + 8, 0).WithLeft(MODEL_AXE + 8, 0);
    definitions[87] = MonsterDefinition(328, SpawnLoader::Monster, 58).WithScale(0.8f);
    definitions[88] = MonsterDefinition(327, SpawnLoader::Monster, 57)
        .WithScale(1.19f)
        .WithRight(MODEL_MACE + 6)
        .WithInitializer(InitializeMonster88);
    definitions[89] = MonsterDefinition(332, SpawnLoader::Monster, 62).WithScale(1.2f).WithRight(MODEL_STAFF, 11);
    definitions[90] = MonsterDefinition(317, SpawnLoader::Monster, 47).WithScale(1.1f);
    definitions[91] = MonsterDefinition(316, SpawnLoader::Monster, 46).WithScale(1.1f).WithLeft(MODEL_BOW + 3, 1);
    definitions[92] = MonsterDefinition(329, SpawnLoader::Monster, 59).WithScale(1.0f).WithRight(MODEL_AXE + 8, 0).WithLeft(MODEL_AXE + 8, 0);
    definitions[93] = MonsterDefinition(328, SpawnLoader::Monster, 58).WithScale(0.8f);
    definitions[94] = MonsterDefinition(327, SpawnLoader::Monster, 57)
        .WithScale(1.19f)
        .WithRight(MODEL_MACE + 6)
        .WithInitializer(InitializeMonster88);
    definitions[95] = MonsterDefinition(332, SpawnLoader::Monster, 62).WithScale(1.2f).WithRight(MODEL_STAFF, 11);
    definitions[96] = MonsterDefinition(317, SpawnLoader::Monster, 47).WithScale(1.1f);
    definitions[97] = MonsterDefinition(316, SpawnLoader::Monster, 46).WithScale(1.1f).WithLeft(MODEL_BOW + 3, 1);
    definitions[98] = MonsterDefinition(329, SpawnLoader::Monster, 59).WithScale(1.0f).WithRight(MODEL_AXE + 8, 0).WithLeft(MODEL_AXE + 8, 0);
    definitions[99] = MonsterDefinition(328, SpawnLoader::Monster, 58).WithScale(0.8f);
    definitions[100] = MonsterDefinition(39, SpawnLoader::None);
    definitions[101] = MonsterDefinition(40, SpawnLoader::None);
    definitions[102] = MonsterDefinition(51, SpawnLoader::None);
    definitions[103] = MonsterDefinition(25, SpawnLoader::None);
    definitions[111] = MonsterDefinition(327, SpawnLoader::Monster, 57)
        .WithScale(1.19f)
        .WithRight(MODEL_MACE + 6)
        .WithInitializer(InitializeMonster88);
    definitions[112] = MonsterDefinition(332, SpawnLoader::Monster, 62).WithScale(1.2f).WithRight(MODEL_STAFF, 11);
    definitions[113] = MonsterDefinition(317, SpawnLoader::Monster, 47).WithScale(1.1f);
    definitions[114] = MonsterDefinition(316, SpawnLoader::Monster, 46).WithScale(1.1f).WithLeft(MODEL_BOW + 3, 1);
    definitions[115] = MonsterDefinition(329, SpawnLoader::Monster, 59).WithScale(1.0f).WithRight(MODEL_AXE + 8, 0).WithLeft(MODEL_AXE + 8, 0);
    definitions[116] = MonsterDefinition(328, SpawnLoader::Monster, 58).WithScale(0.8f);
    definitions[117] = MonsterDefinition(327, SpawnLoader::Monster, 57)
        .WithScale(1.19f)
        .WithRight(MODEL_MACE + 6)
        .WithInitializer(InitializeMonster88);
    definitions[118] = MonsterDefinition(332, SpawnLoader::Monster, 62).WithScale(1.2f).WithRight(MODEL_STAFF, 11);
    definitions[119] = MonsterDefinition(317, SpawnLoader::Monster, 47).WithScale(1.1f);
    definitions[120] = MonsterDefinition(316, SpawnLoader::Monster, 46).WithScale(1.1f).WithLeft(MODEL_BOW + 3, 1);
    definitions[121] = MonsterDefinition(329, SpawnLoader::Monster, 59).WithScale(1.0f).WithRight(MODEL_AXE + 8, 0).WithLeft(MODEL_AXE + 8, 0);
    definitions[122] = MonsterDefinition(328, SpawnLoader::Monster, 58).WithScale(0.8f);
    definitions[123] = MonsterDefinition(327, SpawnLoader::Monster, 57)
        .WithScale(1.19f)
        .WithRight(MODEL_MACE + 6)
        .WithInitializer(InitializeMonster88);
    definitions[124] = MonsterDefinition(332, SpawnLoader::Monster, 62).WithScale(1.2f).WithRight(MODEL_STAFF, 11);
    definitions[125] = MonsterDefinition(317, SpawnLoader::Monster, 47).WithScale(1.1f);
    definitions[126] = MonsterDefinition(316, SpawnLoader::Monster, 46).WithScale(1.1f).WithLeft(MODEL_BOW + 3, 1);
    definitions[127] = MonsterDefinition(329, SpawnLoader::Monster, 59).WithScale(1.0f).WithRight(MODEL_AXE + 8, 0).WithLeft(MODEL_AXE + 8, 0);
    definitions[128] = MonsterDefinition(328, SpawnLoader::Monster, 58).WithScale(0.8f);
    definitions[129] = MonsterDefinition(327, SpawnLoader::Monster, 57)
        .WithScale(1.19f)
        .WithRight(MODEL_MACE + 6)
        .WithInitializer(InitializeMonster88);
    definitions[130] = MonsterDefinition(332, SpawnLoader::Monster, 62).WithScale(1.2f).WithRight(MODEL_STAFF, 11);
    definitions[131] = MonsterDefinition(331, SpawnLoader::Monster, 61).WithScale(0.8f).WithInitializer(InitializeMonster131);
    definitions[132] = MonsterDefinition(330, SpawnLoader::Monster, 60).WithScale(0.8f).WithInitializer(InitializeMonster131);
    definitions[133] = MonsterDefinition(330, SpawnLoader::Monster, 60).WithScale(0.8f).WithInitializer(InitializeMonster131);
    definitions[134] = MonsterDefinition(330, SpawnLoader::Monster, 60).WithScale(0.8f).WithInitializer(InitializeMonster131);
    definitions[150] = MonsterDefinition(302, SpawnLoader::Monster, 32).WithScale(0.12f);
    definitions[151] = MonsterDefinition(310, SpawnLoader::Monster, 40).WithScale(1.3f).WithLeft(MODEL_BOW + 14);
    definitions[200] = MonsterDefinition(236, SpawnLoader::None).WithScale(1.8f).WithBlend(2).WithState446(1);
    definitions[230] = MonsterDefinition(336, SpawnLoader::Npc, 336).WithInitializer(InitializeMonster230);
    definitions[231] = MonsterDefinition(377, SpawnLoader::Npc, 377);
    definitions[232] = MonsterDefinition(375, SpawnLoader::Npc, 375).WithScale(1.0f).WithInitializer(InitializeMonster232);
    definitions[233] = MonsterDefinition(376, SpawnLoader::Npc, 376).WithScale(1.0f).WithInitializer(InitializeMonster232);
    definitions[234] = MonsterDefinition(289, SpawnLoader::Monster, 19)
        .WithScale(1.5f)
        .WithRight(MODEL_STAFF, 4)
        .WithInitializer(InitializeMonster234);
    definitions[235] = MonsterDefinition(374, SpawnLoader::Npc, 374).WithScale(1.0f).WithInitializer(InitializeMonster232);
    definitions[236] = MonsterDefinition(390, SpawnLoader::Npc, 390)
        .WithScale(1.0f)
        .WithSubtype(207)
        .WithState446(8)
        .WithInitializer(InitializeMonster232);
    definitions[237] = MonsterDefinition(349, SpawnLoader::Npc, 349);
    definitions[238] = MonsterDefinition(348, SpawnLoader::Npc, 348).WithBlend(1);
    definitions[239] = MonsterDefinition(347, SpawnLoader::Npc, 347);
    definitions[240] = MonsterDefinition(346, SpawnLoader::Npc, 346);
    definitions[241] = MonsterDefinition(345, SpawnLoader::Npc, 345);
    definitions[242] = MonsterDefinition(343, SpawnLoader::Npc, 343).WithBlend(1).WithInitializer(InitializeMonster242);
    definitions[243] = MonsterDefinition(344, SpawnLoader::Npc, 344);
    definitions[244] = MonsterDefinition(340, SpawnLoader::Npc, 340);
    definitions[245] = MonsterDefinition(342, SpawnLoader::Npc, 342);
    definitions[246] = MonsterDefinition(341, SpawnLoader::Npc, 341);
    definitions[247] = MonsterDefinition(390, SpawnLoader::None)
        .WithRight(MODEL_BOW + 11)
        .WithLeft(MODEL_BOW + 7)
        .WithInitializer(InitializeMonster247);
    definitions[248] = MonsterDefinition(336, SpawnLoader::Npc, 336).WithInitializer(InitializeMonster248);
    definitions[249] = MonsterDefinition(390, SpawnLoader::None).WithRight(MODEL_SPEAR + 7).WithInitializer(InitializeMonster247);
    definitions[250] = MonsterDefinition(336, SpawnLoader::Npc, 336).WithInitializer(InitializeMonster250);
    definitions[251] = MonsterDefinition(338, SpawnLoader::Npc, 338).WithScale(0.95f);
    definitions[253] = MonsterDefinition(337, SpawnLoader::Npc, 337).WithInitializer(InitializeMonster253);
    definitions[254] = MonsterDefinition(339, SpawnLoader::Npc, 339);
    definitions[255] = MonsterDefinition(335, SpawnLoader::Npc, 335).WithInitializer(InitializeMonster255);
    return definitions;
}
constexpr auto MonsterDefinitions = MakeMonsterDefinitions();
constexpr MonsterDefinition DefaultMonsterDefinition{};

// IDA: CreateMonster (0x0045CCF0), escrituras comunes después de CreateCharacter.
unsigned int CreateMonsterFromDefinition(const MonsterDefinition& def, int key, int x, int y)
{
    if (def.Loader == SpawnLoader::Monster) OpenMonsterModel(def.Resource);
    else if (def.Loader == SpawnLoader::Npc) OpenNpc(def.Resource);
    if (def.ExtraResource >= 0) OpenMonsterModel(def.ExtraResource);
    const unsigned int c = CreateChar5(key, def.Model, x, y);
    if (def.Scale > 0.0f) *(float*)(c + 12) = def.Scale;
    if (def.Right >= 0) *(unsigned short*)(c + 624) = (unsigned short)def.Right;
    if (def.Left >= 0) *(unsigned short*)(c + 648) = (unsigned short)def.Left;
    if (def.RightLevel >= 0) *(unsigned char*)(c + 626) = (unsigned char)def.RightLevel;
    if (def.LeftLevel >= 0) *(unsigned char*)(c + 650) = (unsigned char)def.LeftLevel;
    if (def.RightExcellent >= 0) *(unsigned char*)(c + 627) = (unsigned char)def.RightExcellent;
    if (def.BlendMesh != KeepSpawnValue) *(int*)(c + 100) = def.BlendMesh;
    if (def.BlendLight >= 0.0f) *(float*)(c + 104) = def.BlendLight;
    if (def.Subtype >= 0) *(int*)(c + 4) = def.Subtype;
    if (def.HiddenMesh != KeepSpawnValue) *(int*)(c + 88) = def.HiddenMesh;
    if (def.State446 >= 0) *(unsigned short*)(c + 446) = (unsigned short)def.State446;
    if (def.Flag766 >= 0) *(unsigned char*)(c + 766) = (unsigned char)def.Flag766;
    if (def.Initialize) def.Initialize(c);
    return c;
}
} // namespace

// Creación por definición vanilla o por modelo del catálogo.
// Se conserva el quinto parámetro por compatibilidad ABI con los callers existentes.
// IDA: CreateMonster (0x0045CCF0)
char* __cdecl CreateMonster(unsigned int Type_, int PositionX, int PositionY,
                            int Key, int /*phantom_unused*/)
{
    int Type = (int)Type_;
    unsigned int c = 0;
    int v8 = 0;
    int v9, v10;

    // DESVIACION (DLL CustomMonster, ahora catálogo 0.97.20): un monstruo con
    // modelo propio en el server se crea con ese modelo en vez de la definición vanilla.
    const CatalogMonster* custom = gContentCatalog.GetMonster(Type);
    if (custom && custom->Model >= 0) {
        c = CreateCharacter(Key, custom->Model, (unsigned char)PositionX, (unsigned char)PositionY, 0.0f);
        if (!c) return nullptr;
        *(float*)(c + 12) = (custom->Scale > 0.0f) ? custom->Scale : 1.0f;
        const char* name = getMonsterName(Type);
        if (!name || !name[0]) name = custom->Name;
        strncpy_s((char*)(uintptr_t)(c + 0x1c1), 32, name, _TRUNCATE);
        *(unsigned char*)(c + 747) = (unsigned char)Type;
        *(unsigned short*)(c + 8) = 0;
        *(unsigned short*)(c + 784) = (unsigned short)HeroIndex;
        *(unsigned char*)(c + 132) = (custom->Kind == 0) ? 4 : 2;   // 4 NPC, 2 monstruo
        return (char*)(uintptr_t)c;
    }

    const MonsterDefinition& definition = (Type_ < MonsterDefinitions.size())
        ? MonsterDefinitions[Type_] : DefaultMonsterDefinition;
    c = CreateMonsterFromDefinition(definition, Key, PositionX, PositionY);

    // Ajuste de escala por mundo para los tipos 84..136; se conserva el límite original.
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
    if (c && custom && custom->Scale > 0.0f) *(float*)(c + 12) = custom->Scale;

    if (c) {
        // Copia el nombre desde la tabla MonsterScript/NPCName (getMonsterName por
        // Type) a c+0x1c1 (449), que lee Target_Render al hacer hover sobre el NPC/mob.
        // Usa el Type ORIGINAL del packet (el blacksmith es 251), no el model type resuelto.
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
