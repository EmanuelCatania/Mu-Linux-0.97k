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
//     return DAT_07abf5d0 + CHARACTER_STRIDE * 400 + 0x0          // sentinel: puntero al último slot+1
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
//     // Registra entity[+0x388] (PositionX) en HashTable
//     HashTable_GetIndex(entity + 0x388)
//     if not found: new node, add to table
//     entity[+0x388] = entity_type & 0xFF   // escribe type en low byte
//
//     // Registra entity[+0x38C] (PositionY) en HashTable
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
//   void Entity_ClearFlag(ushort flag, intptr_t entity):
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
#include "game/entity/EntityName.h"
#include "platform/Text.h"
#include <array>
#include "assets/data/ContentCatalog.h"
#include "game/physics/ClothTypes.h"
#include "game/monster/Monster.h"
#include "game/monster/MonsterSpawnTable.h"
#include <string>

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
// Devuelve la dirección completa dentro de CharactersClient (DAT_07abf5d0).
CHARACTER* __cdecl CreateCharacter(int Key, int Type, unsigned char PosX,
                                   unsigned char PosY, float Rotation)
{
    BYTE* c = reinterpret_cast<BYTE*>(DAT_07abf5d0);
    for (int i = 0; i < 400; ++i) {
        if (*(unsigned char*)c != 0 && *(short*)&((CHARACTER*)(uintptr_t)(c))->Key == (short)Key) {
            CreateCharacterPointer((unsigned char*)c, Type, PosX, PosY, Rotation);
            return AsCharacter(c);
        }
        c += CHARACTER_STRIDE;
    }
    // search again for an inactive slot
    int v7 = 0;
    BYTE* i = reinterpret_cast<BYTE*>(DAT_07abf5d0);
    while (*(unsigned char*)i != 0) {
        if (++v7 >= 400) {
            // all slots occupied — return sentinel slot at end (matches IDA)
            return AsCharacter(DAT_07abf5d0 + 400 * CHARACTER_STRIDE);
        }
        i += CHARACTER_STRIDE;
    }
    // La limpieza recibe la identidad nativa; los pools se migran por separado.
    DeleteCloth(AsCharacter(i), AsObject(i), nullptr);  // DeleteCloth
    CreateCharacterPointer((unsigned char*)i, Type, PosX, PosY, Rotation);
    *(short*)&((CHARACTER*)(uintptr_t)(i))->Key = (short)Key;
    return AsCharacter(i);
}

// Adaptador usado por las definiciones de monstruos; conserva la dirección.
static inline CHARACTER* CreateChar5(int Key, int Type, int PosX, int PosY)
{
    return CreateCharacter(Key, Type, (unsigned char)PosX, (unsigned char)PosY, 0.0f);
}


extern "C" void DbgLogPublic(const char* msg);

namespace {
// IDA: CreateMonster (0x0045CCF0). Los datos de cada tipo están en
// Content/monster/spawn.toml (game/monster/MonsterSpawnTable.h); acá quedan
// los ajustes que no entran en esos campos, que el .toml nombra con `init`.
using SpawnInitializer = void (*)(CHARACTER*);

// IDA: CreateMonster (0x0045CCF0), monstruos que crean el par de joints 1258.
void InitializeMonsterJoints(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    // El owner del joint aún es de 32 bits; su layout queda para B2.
    Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 2, (intptr_t)c, 30.0f, -1, 0);
    Joint_Create(1258, (float*)(c + 16), (float*)(c + 16), (float*)(c + 28), 3, (intptr_t)c, 30.0f, -1, 0);
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 8.
void InitializeMonster8(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned int*)(c + 120) = 1;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 11.
void InitializeMonster11(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    AsCharacter(c)->Object.AlphaTarget = 0.4f;
    AsCharacter(c)->MoveSpeed = 15;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 25.
void InitializeMonster25(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    AsCharacter(c)->Object.LightEnable = 0;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 34.
void InitializeMonster34(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned short *)&AsCharacter(c)->BodyPart[1].Type = MODEL_HELM + 3;
    *(unsigned short *)&AsCharacter(c)->BodyPart[2].Type = MODEL_ARMOR + 3;
    *(unsigned short *)&AsCharacter(c)->BodyPart[3].Type = MODEL_PANTS + 3;
    *(unsigned short *)&AsCharacter(c)->BodyPart[4].Type = MODEL_GLOVES + 3;
    *(unsigned short *)&AsCharacter(c)->BodyPart[5].Type = MODEL_BOOTS + 3;
    AsCharacter(c)->BodyPart[1].Level = 9;
    AsCharacter(c)->BodyPart[2].Level = 9;
    AsCharacter(c)->BodyPart[3].Level = 9;
    AsCharacter(c)->BodyPart[4].Level = 9;
    AsCharacter(c)->BodyPart[5].Level = 9;
    AsCharacter(c)->PK = 6;
    SetCharacterScale(AsCharacter(c));
    if (gMapManager.GetCurrentMap() == MAP_DEVIL_SQUARE1) {
        AsCharacter(c)->Object.Scale = 1.3f;
    }
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 42.
void InitializeMonster42(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    AsCharacter(c)->Object.BoundingBoxMax[0] = 200.0f;
    AsCharacter(c)->Object.BoundingBoxMax[1] = 150.0f;
    AsCharacter(c)->Object.BoundingBoxMax[2] = 280.0f;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 73.
void InitializeMonster73(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    auto* meshes = reinterpret_cast<ClothModelMesh*>(
        AsModel(DAT_05828d58 + BMD_STRIDE * AsCharacter(c)->Object.Type)->Meshs);
    if (meshes) {
        meshes[0].Unknown0[0] = 1;
        meshes[1].Unknown0[0] = 0;
        meshes[2].Unknown0[0] = 0;
        meshes[3].Unknown0[0] = 1;
        meshes[4].Unknown0[0] = 1;
    }
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 77.
void InitializeMonster77(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned char*)(uintptr_t)(DAT_05828d58 + BMD_STRIDE * 325 + 0x88 + BMD_FIELD_SHIFT) = 0;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 88.
void InitializeMonster88(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    if ((gMapManager.GetCurrentMap() - 9) / 3) {
        AsCharacter(c)->Weapon[0].Level = 0;
    } else {
        AsCharacter(c)->Weapon[0].Level = 8;
    }
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 131.
void InitializeMonster131(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    AsCharacter(c)->Object.EnableShadow = 0;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 230.
void InitializeMonster230(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned short *)&AsCharacter(c)->BodyPart[1].Type = 360;
    *(unsigned short *)&AsCharacter(c)->BodyPart[2].Type = 363;
    *(unsigned short *)&AsCharacter(c)->BodyPart[4].Type = 365;
    *(unsigned short *)&AsCharacter(c)->BodyPart[5].Type = 366;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 232.
void InitializeMonster232(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    AsCharacter(c)->Object.Kind = 4;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 234.
void InitializeMonster234(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    AsCharacter(c)->Object.Kind = 4;
    SetAction(AsObject(c), 0);
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 242.
void InitializeMonster242(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    float v13 = AsCharacter(c)->Object.Position[0];
    float v14 = AsCharacter(c)->Object.Position[1];
    AsCharacter(c)->Object.Position[2] = RequestTerrainHeight(v13, v14) + 140.0f;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 247.
void InitializeMonster247(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned short *)&AsCharacter(c)->BodyPart[1].Type = MODEL_HELM + 9;
    *(unsigned short *)&AsCharacter(c)->BodyPart[2].Type = MODEL_ARMOR + 9;
    *(unsigned short *)&AsCharacter(c)->BodyPart[3].Type = MODEL_PANTS + 9;
    *(unsigned short *)&AsCharacter(c)->BodyPart[4].Type = MODEL_GLOVES + 9;
    *(unsigned short *)&AsCharacter(c)->BodyPart[5].Type = MODEL_BOOTS + 9;
    SetCharacterScale(AsCharacter(c));
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 248.
void InitializeMonster248(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned short *)&AsCharacter(c)->BodyPart[1].Type = 361;
    *(unsigned short *)&AsCharacter(c)->BodyPart[2].Type = 363;
    *(unsigned short *)&AsCharacter(c)->BodyPart[4].Type = 365;
    *(unsigned short *)&AsCharacter(c)->BodyPart[5].Type = 367;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 250.
void InitializeMonster250(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned short *)&AsCharacter(c)->BodyPart[1].Type = 360;
    *(unsigned short *)&AsCharacter(c)->BodyPart[2].Type = 362;
    *(unsigned short *)&AsCharacter(c)->BodyPart[4].Type = 364;
    *(unsigned short *)&AsCharacter(c)->BodyPart[5].Type = 366;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 253.
void InitializeMonster253(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned short *)&AsCharacter(c)->BodyPart[1].Type = 368;
    *(unsigned short *)&AsCharacter(c)->BodyPart[2].Type = 370;
    *(unsigned short *)&AsCharacter(c)->BodyPart[3].Type = 372;
}

// IDA: CreateMonster (0x0045CCF0), configuración especial del tipo 255.
void InitializeMonster255(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    *(unsigned short *)&AsCharacter(c)->BodyPart[1].Type = 351;
    *(unsigned short *)&AsCharacter(c)->BodyPart[2].Type = 353;
    *(unsigned short *)&AsCharacter(c)->BodyPart[3].Type = 355;
    *(unsigned short *)&AsCharacter(c)->BodyPart[5].Type = 359;
}

// Tabla indexada por tipo vanilla; los huecos y los tipos extendidos sin modelo propio
// conservan el caso default. El nombre sigue saliendo de getMonsterName/MonsterScript,
// sin reintroducir los strings fijos omitidos por el port. Los inicializadores se ejecutan después de los datos comunes.
struct NamedInitializer {
    const char*      name;
    SpawnInitializer function;
};

constexpr NamedInitializer kInitializers[] = {
    { "MonsterJoints", InitializeMonsterJoints },
    { "Monster8", InitializeMonster8 },
    { "Monster11", InitializeMonster11 },
    { "Monster25", InitializeMonster25 },
    { "Monster34", InitializeMonster34 },
    { "Monster42", InitializeMonster42 },
    { "Monster73", InitializeMonster73 },
    { "Monster77", InitializeMonster77 },
    { "Monster88", InitializeMonster88 },
    { "Monster131", InitializeMonster131 },
    { "Monster230", InitializeMonster230 },
    { "Monster232", InitializeMonster232 },
    { "Monster234", InitializeMonster234 },
    { "Monster242", InitializeMonster242 },
    { "Monster247", InitializeMonster247 },
    { "Monster248", InitializeMonster248 },
    { "Monster250", InitializeMonster250 },
    { "Monster253", InitializeMonster253 },
    { "Monster255", InitializeMonster255 },
};

SpawnInitializer FindInitializer(const std::string& name)
{
    if (name.empty()) return nullptr;
    for (const NamedInitializer& entry : kInitializers)
        if (name == entry.name) return entry.function;
    static std::string warned;
    if (warned != name) {
        warned = name;
        DbgLogPublic(("MonsterSpawn: init desconocido: " + name).c_str());
    }
    return nullptr;
}

// IDA: CreateMonster (0x0045CCF0), escrituras comunes después de CreateCharacter.
CHARACTER* CreateMonsterFromDefinition(const MonsterSpawn& def, int key, int x, int y)
{
    if (def.Loader == SpawnLoader::Monster) OpenMonsterModel(def.Resource);
    else if (def.Loader == SpawnLoader::Npc) OpenNpc(def.Resource);
    if (def.ExtraResource >= 0) OpenMonsterModel(def.ExtraResource);
    CHARACTER* const c = CreateChar5(key, def.Model, x, y);
    if (def.Scale > 0.0f) AsCharacter(c)->Object.Scale = def.Scale;
    if (def.Right >= 0) *(unsigned short *)&AsCharacter(c)->Weapon[0].Type = (unsigned short)def.Right;
    if (def.Left >= 0) *(unsigned short *)&AsCharacter(c)->Weapon[1].Type = (unsigned short)def.Left;
    if (def.RightLevel >= 0) AsCharacter(c)->Weapon[0].Level = (unsigned char)def.RightLevel;
    if (def.LeftLevel >= 0) AsCharacter(c)->Weapon[1].Level = (unsigned char)def.LeftLevel;
    if (def.RightExcellent >= 0) AsCharacter(c)->Weapon[0].Option1 = (unsigned char)def.RightExcellent;
    if (def.BlendMesh != KeepSpawnValue) AsCharacter(c)->Object.BlendMesh = def.BlendMesh;
    if (def.BlendLight >= 0.0f) AsCharacter(c)->Object.BlendMeshLight = def.BlendLight;
    if (def.Subtype >= 0) AsCharacter(c)->Object.SubType = def.Subtype;
    if (def.HiddenMesh != KeepSpawnValue) AsCharacter(c)->Object.HiddenMesh = def.HiddenMesh;
    if (def.State446 >= 0) AsCharacter(c)->Level = (unsigned short)def.State446;
    if (def.Flag766 >= 0) AsCharacter(c)->Blood = (unsigned char)def.Flag766;
    if (const SpawnInitializer initialize = FindInitializer(def.Init)) initialize(c);
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
    BYTE* c = nullptr;
    int v8 = 0;
    int v9, v10;

    // DESVIACION (DLL CustomMonster, ahora catálogo 0.97.20): un monstruo con
    // modelo propio en el server se crea con ese modelo en vez de la definición vanilla.
    const CatalogMonster* custom = gContentCatalog.GetMonster(Type);
    if (custom && custom->Model >= 0) {
        c = reinterpret_cast<BYTE*>(CreateCharacter(Key, custom->Model, (unsigned char)PositionX, (unsigned char)PositionY, 0.0f));
        if (!c) return nullptr;
        AsCharacter(c)->Object.Scale = (custom->Scale > 0.0f) ? custom->Scale : 1.0f;
        const char* name = getMonsterName(Type);
        if (!name || !name[0]) name = custom->Name;
        EntityName::Set(c, name);
        AsCharacter(c)->MonsterIndex = (unsigned char)Type;
        *(unsigned short*)(c + 8) = 0;
        *(unsigned short *)&AsCharacter(c)->TargetCharacter = (unsigned short)HeroIndex;
        AsCharacter(c)->Object.Kind = (custom->Kind == 0) ? 4 : 2;   // 4 NPC, 2 monstruo
        return reinterpret_cast<char*>(c);
    }

    const MonsterSpawn& definition = MonsterSpawn_Get(Type);
    c = reinterpret_cast<BYTE*>(CreateMonsterFromDefinition(definition, Key, PositionX, PositionY));

    // Ajuste de escala por mundo para los tipos 84..136; se conserva el límite original.
    v9 = gMapManager.GetCurrentMap() - 9;
    if (v9 > 0 && v9 <= 7) {
        v10 = Type;
        if (Type >= 84 && Type <= 136) {
            AsCharacter(c)->Level = 0;
            AsCharacter(c)->Object.Scale = (float)((double)(v9 / 3) * 0.05000000074505806 + AsCharacter(c)->Object.Scale);
        }
    } else {
        v10 = Type;
    }
    if (c && custom && custom->Scale > 0.0f) AsCharacter(c)->Object.Scale = custom->Scale;

    if (c) {
        // Copia el nombre desde la tabla MonsterScript/NPCName (getMonsterName por
        // Type), que lee Target_Render al hacer hover sobre el NPC/mob.
        // Usa el Type ORIGINAL del packet (el blacksmith es 251), no el model type resuelto.
        // Antes se copiaban hasta 31 bytes sobre CHARACTER::ID, que mide 25 hasta
        // GuildMarkIndex; EntityName guarda el nombre entero.
        {
            const char* mname = getMonsterName(Type);
            EntityName::Set(c, mname ? mname : "");
        }
        v8 = 0;  // suppress unused-warning

        AsCharacter(c)->MonsterIndex = (unsigned char)v10;
        *(unsigned short*)(c + 8) = 0;  // v18 was uninit stack — defaults to 0
        *(unsigned short *)&AsCharacter(c)->TargetCharacter = (unsigned short)HeroIndex;

        if (v10 == 200) {
            AsCharacter(c)->Object.Kind = 2;
        } else if (v10 > 200) {
            AsCharacter(c)->Object.Kind = 4;
            return reinterpret_cast<char*>(c);
        } else if (v10 < 150 && v10 <= 110 && v10 >= 100) {
            AsCharacter(c)->Object.Kind = 8;
            return reinterpret_cast<char*>(c);
        } else {
            AsCharacter(c)->Object.Kind = 2;
        }
    }
    return reinterpret_cast<char*>(c);
}
