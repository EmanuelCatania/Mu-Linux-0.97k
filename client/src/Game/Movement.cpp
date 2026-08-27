// Movement.cpp
// Sistema de movimiento de entidades — decompile completo
#include "stdafx.h"
#include "Game/Movement.h"
//
// Cubre: sistema de waypoints, interpolación de posición, animaciones de caminata,
//        funciones de ángulo/dirección, y handlers de paquetes de movimiento del servidor.
//
// ─────────────────────────────────────────────────────────────────────────────
// COORDENADAS
// ─────────────────────────────────────────────────────────────────────────────
//
// Sistema dual de coordenadas:
//   Grid  — byte (0..255), representa casilla del mapa 256×256
//   World — float, coordenada 3D para OpenGL
//
// Conversión grid → world:
//   worldX = (float(gridX) + DAT_00552504) * DAT_005524f0
//   DAT_00552504 = 0.5f  (centrado en casilla)
//   DAT_005524f0 = escala global (≈ tamaño de casilla en unidades de render)
//
// Elevación (Z) se obtiene por FUN_004f7500() — lectura del heightmap del mapa.
//
// ─────────────────────────────────────────────────────────────────────────────
// ESTRUCTURA DE ENTIDAD — CAMPOS DE MOVIMIENTO
// ─────────────────────────────────────────────────────────────────────────────
//
//   Stride del array: 0x394 bytes por entidad
//   Base del array:   DAT_07abf5d0 (aleatorizada con rand()%0x80*0x394 al inicio)
//   Entidad del jugador: DAT_07abf5d8 (= base + 0x394)
//
//   Offset  Tipo    Nombre                  Descripción
//   +0x02   short   entity_type             Tipo (0x186=skeleton, 0x129=player, etc.)
//   +0x04   int     entity_id               ID de red (coincide con DAT_05826cac para el jugador)
//   +0x10   float   world_x                 Posición X world (OpenGL)
//   +0x14   float   world_y                 Posición Y world (OpenGL)
//   +0x18   float   world_z                 Elevación Z (del heightmap)
//   +0x24   float   facing_angle            Ángulo de orientación en grados (0..360)
//   +0x34   float   idle_jitter_x           Offset de animación idle (generado con rand)
//   +0x38   float   idle_jitter_y
//   +0x68   float   height_bob              Oscilación vertical de animación
//   +0x78   int     flags                   Flags varios (bit1 = doble velocidad, etc.)
//   +0x105  byte    anim_state              ID de animación actual
//   +0x106  byte    anim_state_prev         Animación anterior (para blending)
//   +0x108  int     anim_timer              Timer de frame de animación actual
//   +0x10c  int     anim_timer_prev         Timer de animación anterior
//   +0x161  char    smooth_angle_flag       0=lerp suave, 1=snap directo
//   +0x164  float   target_angle            Ángulo destino para interpolación
//   +0x168  float   current_smooth_angle    Ángulo suavizado actual
//   +0x1bc  byte    move_type_flags         Bits 0-2: 1=normal, 2=nado, 3=carrera
//   +0x240  short   mount_id                Montura (-1=ninguna)
//   +0x242  byte    mount_speed_tier        Nivel de velocidad de montura
//   +0x258  short   action_target           Objetivo de acción (weapon/NPC slot)
//   +0x25a  byte    action_speed_tier
//   +0x270  short   weapon_item_id          Arma equipada (-1=ninguna)
//   +0x288  short   shield_item_id          Escudo equipado (-1=ninguna)
//   +0x2a0  short   ride_state              Estado de montura (-1=no montado)
//   +0x2b8  short   char_class              Clase (0x332=DK, 0x333=DW, etc.)
//   +0x2ec  byte    move_pending            1=movimiento en cola de acción
//   +0x2fd  byte    locked                  1=entidad bloqueada (no procesar mov.)
//   +0x300  byte    stamina_counter         Contador agotamiento (0..0x28; >=0x28 = cansado)
//   +0x305  byte    move_trigger            1=ejecutar movimiento inmediato
//   +0x306  byte    target_grid_x           Destino X en grid (recibido del servidor)
//   +0x307  byte    target_grid_y           Destino Y en grid
//   +0x310  short   attack_target_index     Índice de entidad objetivo de ataque (-1=ninguno)
//   +0x34e  char    SafeZone                1=parado en zona segura
//   +0x350  char    state_flag              Estado especial de movimiento
//   +0x354  byte    path_current_wp         Índice del waypoint actual (0..N-1)
//   +0x355  byte    path_substep            Sub-paso dentro del waypoint (0..3)
//   +0x356  byte    path_wp_count           Total de waypoints en el path actual
//   +0x357  byte[15] path_wp_x             Coordenadas X de waypoints (grid, 15 max)
//   +0x366  byte[15] path_wp_y             Coordenadas Y de waypoints (grid, 15 max)
//   +0x388  int     cached_wp_x             Caché de coordenada X del waypoint actual (grid)
//   +0x38c  int     cached_wp_y             Caché de coordenada Y del waypoint actual
//
// ─────────────────────────────────────────────────────────────────────────────
// IDs DE ANIMACIÓN (entity+0x105)
// ─────────────────────────────────────────────────────────────────────────────
//
//   0x00  Idle (con jitter aleatorio de orientación)
//   0x02  Walk estándar (NPC/mob)
//   0x06  Muerte / no animar
//   0x0D  Walk normal (player sin arma pesada)
//   0x0E  Run / carrera con arma
//   0x0F  Walk con arma ligera
//   0x10  Walk con arma pesada (spear/staff)
//   0x11  Run montado
//   0x12  Run con arma
//   0x13  Walk con escudo
//   0x14  Walk doble arma
//   0x15  Walk rápido (monte)
//   0x16  Walk agotado (stamina >= 0x28)
//   0x17  Walk en agua (nado superficial)
//   0x18  Nado con ambas manos
//   0x1B  Walk mago con escudo
//   0x1C  Walk con escudo alternativo
//   0x1D  Walk super-agotado
//   0x1E  Walk con objetivo en rango
//   0x1F  Walk con objetivo en rango + escudo
//   0x20  DK idle (sin ataque activo)
//   0x21  DK walk con ataque
//   0x4C  DW idle (g_GameSubState=8 o 10, sin objetivo)
//   0x4D  DW walk con objetivo
//   0x4E  Walk arma especial 0x1AF
//   0x4F  Walk arma 0x1AF con objetivo
//   0x50  Walk mago con escudo y arma especial
//
// ─────────────────────────────────────────────────────────────────────────────
// FUNCIONES DE ÁNGULO Y DIRECCIÓN
// ─────────────────────────────────────────────────────────────────────────────

// 0x0043e050  Entity_GetDirCode(float x1, float y1, float x2, float y2) → ushort
//   Calcula el código de dirección (ángulo) desde (x1,y1) hacia (x2,y2).
//   Usa fpatan (atan2 x87) para la dirección real.
//   Retorna ushort con bits de comparación del FPU (formato interno de ángulo).
//   Llamado por: Entity_FaceTarget, Entity_MoveAlongPath, PacketHandler_0x10.
float Entity_GetDirCode(float x1, float y1, float x2, float y2);

// 0x0043e1b0  Angle_ShortestLerp(float current, float target, float step) → float
//   Interpolación suavizada del ángulo más corto entre current y target.
//   Maneja el wrap-around de 360° correctamente.
//   DAT_005524ec = 180.0f (punto de inflexión del wrap)
//   DAT_0055286c = 360.0f
//   Si current >= 180: intenta reducir wrap-around por ambos lados.
//   Retorna el ángulo interpolado (float10 → float).
float Angle_ShortestLerp(float current, float target, float step);

// 0x0043e370  Angle_Delta(float a, float b, char abs_flag) → float
//   Calcula la diferencia angular mínima entre a y b (con wrap 360°).
//   Si abs_flag==1: retorna valor absoluto (siempre positivo).
//   Normaliza ambos ángulos a [0,360) antes de calcular.
float Angle_Delta(float a, float b, char abs_flag);

// 0x0043e4a0  Entity_UpdateFacing(float* pos, float* entity, float* target, float step)
//   Llama Entity_GetDirCode(pos, target) → dirCode
//   Llama Angle_ShortestLerp(entity[2], dirCode, step) → entity[2] (ángulo)
//   Calcula distancia 3D y actualiza ángulo de elevación.
void Entity_UpdateFacing(float* pos, float* entity_angle, float* target, float step);

// 0x0043e570  Entity_ApplyRotation(float* out, float* quat, float* vec)
//   FUN_004f9db0(quat, mat4x3)  — quaternion → matriz rotación 4×3
//   FUN_004fa0b0(vec, mat4x3, &local)  — matriz × vector → local
//   out[0..2] += local[0..2]  (aplica la rotación al offset out)
void Entity_ApplyRotation(float* out, float* quat, float* vec);

// 0x0043e5c0  Entity_SmoothAngle(int entity)
//   Lee entity+0x161 (flag):
//     0 → lerp suave: entity+0x168 += (entity+0x164 - entity+0x168) * DAT_005524f4
//   Flag ≠ 0 → avanza/retrocede DAT_00552874 directamente (snap incremental).
//   Clamp a [0,1] (DAT_0055256c = 1.0f).
//   Actualiza entity+0x68 si entity+0x168 < entity+0x68.
void Entity_SmoothAngle(int entity);

// 0x0043e680  Entity_SteerAvoid(int entity, int self_idx, int entity_array, int count)
//   Steering de evasión: itera el array de entidades (stride 0x1BC).
//   Para cada entidad dentro de DAT_00552850 unidades:
//     Calcula vector de repulsión ponderado.
//   Promedia todos los vectores de repulsión.
//   Llama Entity_GetDirCode y Angle_ShortestLerp para actualizar entity+0x24.
void Entity_SteerAvoid(int entity, int self_idx, int entity_array, int count);

// ─────────────────────────────────────────────────────────────────────────────
// FUNCIONES DE ANIMACIÓN
// ─────────────────────────────────────────────────────────────────────────────

// 0x0043e820  Entity_SetAnimation(int entity, uint animId)
//   Valida animId:
//     animId < animData[entity.type * 0xbc + 0x26]  (máximo del tipo)
//     o animId == 0x4C/0x4D (excepciones fijas)
//   Si animId != anim actual (entity+0x105):
//     entity+0x106 = entity+0x105  (guarda anim anterior para blending)
//     entity+0x10c = entity+0x108  (guarda timer anterior)
//     entity+0x105 = animId
//     entity+0x108 = 0             (reinicia timer)
void Entity_SetAnimation(int entity, uint animId);

// 0x0043e890  Entity_FaceTarget(int entity, int target)
//   Si target != 0:
//     Entity_GetDirCode(entity.pos, target.pos) → dirCode
//     Angle_Delta(entity.angle, dirCode, 1) → delta
//     Si delta < DAT_00552848 (umbral):
//       entity+0x34 = entity.angle - dirCode  (jitter X)
//       entity+0x38 = (target.z - entity.z - 0.5) * sin_factor  (jitter Y / elevación)
//       Normaliza ambos a [0, 360)
//     Si delta >= umbral: reset jitter a (0, 0).
void Entity_FaceTarget(int entity, int target);

// 0x0043e940  Entity_AnimTick(int entity)
//   Máquina de estados de animación por frame.
//   Lee entity+0x105 (anim_state):
//     0x06 (muerte): no avanzar animación.
//     0x00 (idle):   con rand()&0x1F==0, asigna jitter aleatorio a entity+0x34/+0x38.
//     0x02 (walk NPC): si entity+0x310 != -1:
//                        Entity_FaceTarget(entity, target_entity)
//     Otros: reset entity+0x34=0, entity+0x38=0.
//   Nota: el avance real de frames de animación está en FUN_00440060 (AnimTimer_Tick).
void Entity_AnimTick(int entity);

// ─────────────────────────────────────────────────────────────────────────────
// MOVIMIENTO POR WAYPOINTS
// ─────────────────────────────────────────────────────────────────────────────

// 0x0043ea20  Entity_MoveAlongPath(int entity, char update_angle) → int
//   Tick de movimiento a lo largo de la lista de waypoints.
//   Retorna 0 si no hay movimiento (path_current_wp >= path_wp_count).
//
//   PATH WAYPOINT SYSTEM:
//     - Hasta 15 waypoints en arrays entity+0x357 (X) y entity+0x366 (Y).
//     - path_current_wp (byte +0x354) = índice del waypoint en curso.
//     - path_substep    (byte +0x355) = sub-paso 0..3 dentro del waypoint.
//     - path_wp_count   (byte +0x356) = número de waypoints total.
//
//   INTERPOLACIÓN (4 sub-pasos por waypoint):
//     Para cada sub-paso 0..3 se calcula la posición objetivo como combinación
//     lineal de los 4 waypoints circundantes con diferentes coeficientes
//     (constantes DAT_00552880..0x52888C — spline cúbica de Catmull-Rom o similar).
//     Casos por entity+0x355:
//       0: coef (0x52880, 0x5288c, 0x52888, 0x52884)
//       1: coef (0x5287c, 0x5287c, 0x526dc, 0x526dc)
//       2: coef (0x5288c, 0x52884, 0x52880, 0x52888)
//       3: posición directa del waypoint (sin interpolación)
//
//   AVANCE:
//     - Si distancia al sub-objetivo < DAT_005524fc (epsilon):
//         path_substep += 1
//         Si path_substep >= 4:
//             path_substep = 0
//             path_current_wp += 1
//     - Al llegar al último waypoint: marcar entidad como estacionaria.
//
//   ACTUALIZACIÓN DE ÁNGULO (si update_angle != 0):
//     Entity_GetDirCode(entity.pos, target_pos) → dirCode
//     Angle_Delta(entity+0x24, dirCode, 1) → delta
//     Si delta >= DAT_00552844: Angle_ShortestLerp(entity+0x24, dirCode, delta*0.5)
//     Si delta < threshold:     entity+0x24 = dirCode  (snap)
//
//   HASH TABLE ANTI-TAMPER:
//     Las coordenadas cached en entity+0x388/+0x38c se pasan por la misma
//     rutina HashTable_GetIndex/Insert que en otros lugares del código.
//     Es obfuscación de posición; las coordenadas reales están en +0x10/+0x14.
int Entity_MoveAlongPath(int entity, char update_angle);

// ─────────────────────────────────────────────────────────────────────────────
// SELECCIÓN DE ANIMACIÓN DE CAMINATA
// ─────────────────────────────────────────────────────────────────────────────

// 0x004430c0  Entity_SetWalkAnimation(int entity)
//   Determina y aplica el ID de animación de caminata correcta según contexto.
//   Llama FUN_004f6c40(x, y) = GetTileType para detectar agua/terreno especial.
//   DAT_07d78068 = tabla de propiedades de tiles (flags de terreno, stride 0x40).
//   Lógica de selección:
//     entity_type == 0x186 → animaciones especiales (skip; ver Entity_UpdateWalkAnim)
//     entity+0x34e (dead)  → skip
//     entity+0x2b8 (class 0x332/0x333):
//       sin objetivo: animId = 0x0B (DK idle) o 0x0C
//       con objetivo: animId = 0x0C
//     entity+0x2a0 != -1 (montado):
//       con objetivo en rango [0x218..0x21F] o 0x220/0x222: animId = 0x0A
//       sin objetivo: animId = 0x09
//     entity+0x1bc bits 0-2:
//       2 → nado: animId = 0x02
//       1 → caminata: selección por arma/escudo (0x0D..0x14, ver tabla arriba)
//     Exhausto (entity+0x300 >= 0x28): animId = 0x16
//   Aplica con Entity_SetAnimation(entity, animId).
//   Tras setear anim, con rand()&0xF==0 llama FUN_00404bc0(soundId, entity, 0) — sonido de paso.
void Entity_SetWalkAnimation(int entity);

// 0x00443930  Entity_UpdateWalkAnim(int entity)
//   Versión extendida de Entity_SetWalkAnimation para entidad type==0x186 (y otras).
//   Gestiona adicionalmente:
//     entity+0x300 (stamina): si move_type_flags bit2==1 (fast), incrementa contador.
//     Si entity_type == 0x186: inicializa tabla de bone weights en DAT_05828d58+0x11E98:
//       Offsets 0xD0..0x140 (step 0x10): peso = 0x3EA8F5C3 (≈ 0.083f)
//       Offsets 0x160..0x210 (step 0x10): peso = 0x3EAE147B (≈ 0.085f)
//       Si entity+0x78 bit1==1: multiplica por DAT_00552504 (0.5f)
//     Luego, lógica completa de selección de animación (similar a Entity_SetWalkAnimation)
//     con más casos para estados de juego (g_GameSubState==7, 8, 10).
//     Finaliza con Entity_SetAnimation + FUN_00404bc0 para sonidos.
void Entity_UpdateWalkAnim(int entity);

// ─────────────────────────────────────────────────────────────────────────────
// HANDLERS DE PAQUETES DE MOVIMIENTO DEL SERVIDOR
// ─────────────────────────────────────────────────────────────────────────────

// 0x00427b90  PacketHandler_0x10(int packet)
//   Paquete: [C1][len][0x10][entityId_hi][entityId_lo][gridX][gridY][speed_flags]
//   Actualiza posición destino de la entidad.
//
//   entity = FUN_0045ac80(byte[3]<<8 | byte[4])  — busca entidad por ID de red
//   entity+0x2fc = byte[7] >> 4                   — speed tier
//
//   SI es el jugador (entityId == DAT_05826cac):
//     Actualiza entity+0x388 = byte[5] (X grid start)
//     Actualiza entity+0x38c = byte[6] (Y grid start)
//     Si entity+0x2ec == 0: enqueue inmediato.
//
//   SI otra entidad:
//     entity+0x306 = byte[5]  (target X grid)
//     entity+0x307 = byte[6]  (target Y grid)
//     Caso especial type==0x142 (objeto/efecto especial):
//       Actualiza entity+0x388/38c, calcula ángulo, llama Entity_UpdateWalkAnim.
//       entity+0x305 = 1 (trigger inmediato).
//     Caso normal (entity+0x350 == 0):
//       FUN_0043d3e0 / FUN_0043f3e0 → PacketQueue_Enqueue con coordenadas
//       Si cola disponible: entity+0x2ec = 1 (movimiento en cola)
//       Else: entity+0x2ec = 0, FUN_004430c0 (ejecutar animación inmediata)
void PacketHandler_0x10(int packet);

// 0x00427f40  PacketHandler_0x11(int packet)
//   Paquete: [C1][len][0x11][entityId_hi][entityId_lo][gridX][gridY]
//   Teleport / warp o actualización directa de posición (sin path).
//
//   Caso especial type==0xEC (flecha/proyectil):
//     entity+0xD8 = 20.0f (velocidad inicial)
//     entity+0xC4 = (byte[5] - entity.x) * DAT_00552850  (velocidad X)
//     entity+0xC0 = (entity.y - byte[6]) * DAT_00552850  (velocidad Y)
//     FUN_00404bc0(0x1B, ...) — notificación UI
//     FUN_004660f0(entity+0x10, 0) — aplicar fuerza/posición
//   Caso general:
//     entity+0x388 = byte[5]  (target X grid)
//     entity+0x38c = byte[6]  (target Y grid)
//     entity+0x306 = byte[5]
//     entity+0x307 = byte[6]
//     entity+0x305 = 1       (trigger movimiento)
void PacketHandler_0x11(int packet);

// 0x00427a00  PacketHandler_0x0D(int packet)
//   Paquete de chat/notificación del servidor (NO es movimiento).
//   byte[3] sub-tipo:
//     0: UI_AddNotice(packet+4, 0)      — broadcast/aviso de sistema
//     1: FUN_00480620(..., packet+4, 1) — mensaje de GM
//     2: Copia string de packet+4 a DAT_07e913a8 (max 14 chars)
//                                        para banner de texto en pantalla
void PacketHandler_0x0D(int packet);

// ─────────────────────────────────────────────────────────────────────────────
// OTRAS FUNCIONES AUXILIARES DE MOVIMIENTO (documentadas en Net_Process.cpp)
// ─────────────────────────────────────────────────────────────────────────────
//
//   0x0043f3e0  PacketQueue_Enqueue(uint id, float current, uint x, uint y, void* pathState, float t)
//     Wrapper → FUN_0043f500(DAT_05826df4, id, t, x, y, 1, 2, t)
//     Inserta movimiento en la cola de acciones de red (predicción de cliente).
//
//   0x0043f500  ActionQueue_Insert(this, int id, float t, int x, int y, int p5, int p6, float p7)
//     Cola circular de acciones de movimiento con timestamps.
//     Gestiona overwrite de slots por prioridad.
//
//   0x0043fd30  Queue_SetRange(this, int param)
//     Control del rango circular del buffer de acciones.
//     Actualiza write ptr (this+0x400) y read ptr (this+0x404).
//
//   0x0043fd70  AnimTimer_Update(void)
//     Inicia/actualiza timer global de animación con timeGetTime().
//     DAT_05826e04 = timestamp base; DAT_05826e10 = delta por frame.
//     Usado por Entity_AnimTick para avanzar frames.
//
//   0x004f6c40  GetTileType(uint x, uint y) → int
//     Devuelve tipo de tile del mapa en posición grid (x, y).
//     DAT_0838bc70 = tabla de atributos de tile del mapa cargado.
//     Bit 0 = walkable, bit otros = agua, obstáculo, etc.
//
//   0x0045ac80  Entity_FindByNetId(uint netId) → int
//     Busca el índice en el array de entidades por ID de red (0..0x7FFF).
//     Retorna índice (0-based); entidad = DAT_07abf5d0 + ret * 0x394.
//
// ─────────────────────────────────────────────────────────────────────────────
// POOLS DE ENTIDADES (identificados en este análisis)
// ─────────────────────────────────────────────────────────────────────────────
//
//   DAT_07abf5d0  Main entity array        stride 0x394, ≥8 entidades (login scene)
//   DAT_0839be18  Render entity pool A     stride 0x1BC, 12 entidades (visible list)
//   DAT_083a2e92  Render entity pool B     stride 0x1BC, 10 entidades (NPC/effect)
//   DAT_07abf5d8  Player entity            = base + 0x394 (primer slot)
//
// ─────────────────────────────────────────────────────────────────────────────
// FLUJO COMPLETO DE UN PASO DE MOVIMIENTO
// ─────────────────────────────────────────────────────────────────────────────
//
//   1. Servidor envía C1 0x10 [entityId][gridX][gridY][speed]
//   2. PacketHandler_0x10 actualiza entity+0x306/307 (destino grid)
//   3. PacketQueue_Enqueue inserta acción en cola (o ejecuta inmediato)
//   4. Game_MainLoop → Game_SceneUpdate → Entity_MoveAlongPath(entity, 1)
//      por cada entidad activa en el tick de juego
//   5. Entity_MoveAlongPath avanza path_substep; calcula posición world interpolada
//   6. Entity_SetWalkAnimation selecciona animId según equipo/terreno/estado
//   7. Entity_SetAnimation aplica animId con blending desde anim anterior
//   8. Entity_AnimTick genera efectos secundarios (jitter idle, face target)
//   9. En el render: Entity_PrepareRender → Sprite_Draw usa entity+0x10/14/24
//                    para posicionar y orientar el sprite/modelo
