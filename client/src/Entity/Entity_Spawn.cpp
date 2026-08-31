// Entity_Spawn.cpp — FUN_0045adc0 @ 0x0045adc0 (CreateCharacterPointer)
// Initialize/spawn an entity.
//
// Ghidra-confirmed signature:
//   void __cdecl CreateCharacterPointer(CHARACTER* c, int Type,
//                                       int PositionX, int PositionY,
//                                       float Rotation)
//   c         = entity pointer (stride 0x394 from DAT_07abf5d0)
//   Type      = object type code → entity[+0x02] (e.g. 0x186 for hero placeholder)
//   PositionX = grid X coordinate → entity[+0x388]
//   PositionY = grid Y coordinate → entity[+0x38c]
//   Rotation  = initial facing angle (stored into Object.Angle)
//
// ── Hash-table obfuscation note ───────────────────────────────────────────────
// Lines 1-480 (of 797) of the Ghidra decompile consist almost entirely of
// HashTable encode/decode operations on (param_1+0x388) and (param_1+0x38c)
// — the two grid coordinate slots. The anti-tamper XOR-encodes those words,
// then decodes them immediately to use in the terrain-attribute test below.
// Per CLAUDE.md policy these are reference-count obfuscation, not game logic.
// The effective result of those 480 lines is:
//   param_1[0x388] = grid_x  (param_2 & 0xff)
//   param_1[0x38c] = grid_y  ((param_2 >> 8) & 0xff)
// — exactly what we write directly here.
//
// Lines 481-797 contain the real initialization logic, fully implemented below.
//
// ── Entity struct offsets used ────────────────────────────────────────────────
// +0x00  = active flag (byte, 1=active)
// +0x02  = entity_type (short)
// +0x04  = unk_04 (int)
// +0x0c  = scale base (float, init 0.9)
// +0x10  = world_x (float)
// +0x14  = world_y (float)
// +0x18  = world_z (float)
// +0x1bc = move_type_flags (byte)
// +0x1c  = unk_1c
// +0x1da = unk_1da (short, init 0xffff)
// +0x1e2 = equip slot 0..5 (stride 0x18, 6 slots)
// +0x272 = equip slot 6..7 (stride 0x18, 2 slots)
// +0x2a0 = unk_2a0 (short, init 0xffff)
// +0x2b8 = char_class (short, init 0xffff)
// +0x2d0 = unk_2d0 (short, init 0xffff)
// +0x2e9 = unk_2e9 (byte)
// +0x2ea = unk_2ea (byte, init 3)
// +0x2eb = tipo de monstruo (byte).  CreateCharacterPointer lo deja en 0xFF
//          (= sin tipo, el valor del heroe) y CreateMonster escribe el Type real.
// +0x2ec = unk_2ec (byte)
// +0x2ed = unk_2ed (implicit)
// +0x2fa = unk_2fa (short, init 10)
// +0x2fd = render_visible (byte)
// +0x2fe = unk_2fe (byte)
// +0x300 = stamina_counter (byte)
// +0x305 = unk_305 (byte)
// +0x306 = target_grid_x (byte)
// +0x307 = target_grid_y (byte)
// +0x330 = unk_330 (int, init 0xffffffff)
// +0x334 = unk_334 (int)
// +0x33c = unk_33c (int, init 0xffffffff)
// +0x340 = g_framecount mirror (int)
// +0x344 = unk_344
// +0x34e = SafeZone (byte) — TerrainWall[Terrain_Load(x,y)] & 1. NO es dead
//         (el dead real es +0x2FD, IDA ReceiveDie L18).
// +0x34f = unk_34f (byte)
// +0x354 = path_current_wp (byte)
// +0x356 = path_wp_count (byte)
// +0x388 = cached_wp_x (int)
// +0x38c = cached_wp_y (int)
// +0x17c = base_level (int)

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <windows.h>

// Terrain attribute table: 1 byte per attr index, bit 0 = impassable/death flag
// (declared in globals.h as char[]; cast to unsigned char on use)

// ─────────────────────────────────────────────────────────────────────────────

extern "C" { void DbgLogPublic(const char*); }

void __cdecl FUN_0045adc0(unsigned char *param_1, int Type,
                          int PositionX, int PositionY, float Rotation)
{
    // DIAG: trace first 6 calls
    static int s_spawn_dbg = 0;
    if (s_spawn_dbg < 6) {
        char b[160];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "CreateCharacterPointer #%d ptr=%p Type=0x%x PosX=%d PosY=%d Rot=%.2f",
            s_spawn_dbg, param_1, Type, PositionX, PositionY, Rotation);
        DbgLogPublic(b);
        s_spawn_dbg++;
    }

    // ── Anti-tamper block (lines 1-480 in Ghidra) ────────────────────────────
    // HashTable encode/decode on param_1+0x388 and param_1+0x38c.
    // Net result: entity[0x388] = grid_x, entity[0x38c] = grid_y.
    // Omitted per project policy (hash table operations are not game logic).
    // ─────────────────────────────────────────────────────────────────────────

    unsigned int grid_x      = (unsigned int)PositionX;
    unsigned int grid_y      = (unsigned int)PositionY;
    unsigned int entity_type = (unsigned int)Type;

    // Store grid coords into entity (the value the HashTable block also writes)
    *(unsigned int *)(param_1 + 0x388) = grid_x;
    *(unsigned int *)(param_1 + 0x38c) = grid_y;

    // ── Real initialization (lines 481+ in Ghidra) ───────────────────────────

    // Atributo del terreno en la posición de spawn → SafeZone (+0x34E)
    int terrainAttr = FUN_004f6c40((int)grid_x, (int)grid_y);
    if ((unsigned char)DAT_0838bc70[terrainAttr] & 1) {
        param_1[0x34e] = 1;   // spawn dentro de zona segura
    } else {
        param_1[0x34e] = 0;
    }

    // Reset movement/path state
    param_1[0x356] = 0;     // path_wp_count
    param_1[0x354] = 0;     // path_current_wp
    param_1[0x2ec] = 0;     // misc combat flag

    // Mark active + reset anim/combat fields
    param_1[0x00]  = 1;
    param_1[0x160] = 0;
    param_1[0x161] = 1;
    param_1[0xdc]  = 1;
    param_1[0xe4]  = 0;
    param_1[0x110] = 1;
    param_1[0x8c]  = 0;
    param_1[0x2fd] = 0;   // dead_flag (IDA L440 `*(BYTE*)(v5+765) = 0`) — es el
                          // campo que `ReceiveDie` pone en 1; la etiqueta vieja
                          // ("render_visible") era falsa.
    param_1[0x2fe] = 0;
    param_1[0x2e9] = 0;
    param_1[0x300] = 0;   // stamina_counter

    // Entity type and initial values
    *(short  *)(param_1 + 0x1da) = (short)0xffff;       // entity+474 (entity_index) = -1
    param_1[0x2ea]               = 3;
    *(short  *)(param_1 + 2)     = (short)entity_type;
    *(float  *)(param_1 + 0x0c)  = 0.9f;         // 0x3f666666
    *(int    *)(param_1 + 0x80)  = 0;
    *(float  *)(param_1 + 0x168) = 1.0f;          // scale
    *(float  *)(param_1 + 0x164) = 1.0f;
    *(int    *)(param_1 + 0xcc)  = 0;
    *(int    *)(param_1 + 0xd4)  = 0;
    param_1[0x194]               = 0;

    // move_type_flags: clear unless entity type is a swim-type (0xaa..0xad)
    if ((int)entity_type < 0xaa || 0xad < (int)entity_type) {
        param_1[0x1bc] = 0;
    }

    // anim state for hero (Type==390): entity[261]=1, entity[262]=1
    // (IDA L458-466 — only set to 1 when Type==390/0x186; otherwise 0)
    if (entity_type == 0x186) {
        param_1[0x106] = 1;
        param_1[0x105] = 1;
    } else {
        param_1[0x106] = 0;
        param_1[0x105] = 0;
    }

    *(int    *)(param_1 + 0x108) = 0;
    *(int    *)(param_1 + 0x10c) = 0;
    param_1[0x305]               = 0;
    *(int    *)(param_1 + 0x58)  = -1;          // facing / slot = none
    *(short  *)(param_1 + 0x2fa) = 10;
    *(int    *)(param_1 + 0x78)  = 0;
    param_1[0x7c]                = 0;
    param_1[0x84]                = 1;            // alive state
    param_1[0x34f]               = 0;
    *(int    *)(param_1 + 4)     = 0;
    param_1[0x2eb]               = 0xff;
    *(float  *)(param_1 + 0x6c)  = 0.0f;
    *(float  *)(param_1 + 0x70)  = 0.0f;

    // ── World position from grid coords ────────────────────────────────────────
    // Hash table blocks operate on param_1+0x388 and param_1+0x38c again here
    // (same anti-tamper pattern). Omitted.
    *(float *)(param_1 + 0x10) = (float)(int)grid_x * _DAT_005524f0 + _DAT_00552598;
    *(float *)(param_1 + 0x14) = (float)(int)grid_y * _DAT_005524f0 + _DAT_00552598;

    // World Z: terrain height + height offset based on entity type / state
    float terrainH = FUN_004f7500(*(float*)(param_1 + 0x10),
                                  *(float*)(param_1 + 0x14));
    if (DAT_0055a7ac == -1 ||
        *(short *)(param_1 + 0x2b8) != (short)0x333 ||
        param_1[0x34e] != '\0')
    {
        // Normal spawn
    }
    else if (DAT_0055a7ac == 8 || DAT_0055a7ac == 10) {
        terrainH += (float)_DAT_00552848;
    }
    else {
        terrainH += (float)_DAT_0055284c;
    }
    *(float *)(param_1 + 0x18) = terrainH;

    // ── Object.Angle init + Rotation (IDA L640-651) ──────────────────────────
    // entity+0x1c = Angle[0] = 0 (pitch)
    // entity+0x20 = Angle[1] = 0 (roll)
    // entity+0x24 = Angle[2] = Rotation (yaw / facing) ← CRITICAL for char-select
    *(int   *)(param_1 + 0x1c)   = 0;
    *(int   *)(param_1 + 0x20)   = 0;
    *(float *)(param_1 + 0x24)   = Rotation;

    // entity+0x118/0x11c (=280/284): bone offset / pivot for wing render.
    // IDA L641-642: -1032847360 = 0xC2700000 = -60.0f. Set ONCE — don't overwrite.
    *(int    *)(param_1 + 0x118) = 0xc2700000;   // -60.0f
    *(int    *)(param_1 + 0x11c) = 0xc2700000;
    *(int    *)(param_1 + 0x324) = 0;
    *(int    *)(param_1 + 0x328) = 0;
    *(int    *)(param_1 + 0xe8)  = 0;
    *(int    *)(param_1 + 0xec)  = 0;
    *(int    *)(param_1 + 0xf0)  = 0;
    *(int    *)(param_1 + 0x120) = 0;            // entity+288 = 0 (IDA L651)

    // ── Per-Type scale/translation (IDA L652-704) ────────────────────────────
    // entity+0x124 (292) and +0x128 (296) = scale base; +0x12C (300) = scale top.
    // For Type=390 (hero): 40.0f, 40.0f, 120.0f.
    // Default (most monsters): 50.0f, 50.0f, 150.0f.
    switch ((int)entity_type) {
        case 272: case 276: case 279: case 287: case 289: case 290:
            *(int *)(param_1 + 0x124) = 1112014848;
            *(int *)(param_1 + 0x128) = 1112014848;
            *(int *)(param_1 + 0x12c) = 1117782016;
            break;
        case 281: case 301: case 309: case 312: case 314:
            *(int *)(param_1 + 0x124) = 1116471296;
            *(int *)(param_1 + 0x128) = 1116471296;
            *(int *)(param_1 + 0x12c) = 1132068864;
            break;
        case 307:
            *(int *)(param_1 + 0x124) = 1120403456;
            *(int *)(param_1 + 0x128) = 1120403456;
            *(int *)(param_1 + 0x12c) = 1125515264;
            break;
        case 330:
            *(int *)(param_1 + 0x118) = (int)0xC2380000;  // -46.0
            *(int *)(param_1 + 0x11c) = (int)0xC2200000;  // -40.0
            *(int *)(param_1 + 0x120) = 0;
            *(int *)(param_1 + 0x124) = 1119092736;
            *(int *)(param_1 + 0x128) = 1112014848;
            *(int *)(param_1 + 0x12c) = 1128792064;
            break;
        case 331:
            *(int *)(param_1 + 0x120) = 0;
            *(int *)(param_1 + 0x118) = (int)0xC2C00000;
            *(int *)(param_1 + 0x11c) = (int)0xC2C00000;
            *(int *)(param_1 + 0x124) = 1120403456;
            *(int *)(param_1 + 0x128) = 1120403456;
            *(int *)(param_1 + 0x12c) = 1133903872;
            break;
        case 390:  // hero (CRITICAL for char-select rendering)
            // 1109393408 = 0x42200000 = 40.0f
            // 1123024896 = 0x42F00000 = 120.0f
            *(int *)(param_1 + 0x124) = 1109393408;  // 40.0f
            *(int *)(param_1 + 0x128) = 1109393408;  // 40.0f
            *(int *)(param_1 + 0x12c) = 1123024896;  // 120.0f
            break;
        default:
            *(int *)(param_1 + 0x124) = 1112014848;  // 52.0f
            *(int *)(param_1 + 0x128) = 1112014848;
            *(int *)(param_1 + 0x12c) = 1125515264;  // 156.0f
            break;
    }

    *(int    *)(param_1 + 0x17c) = DAT_083a7c00; // base_level from global

    // ── BoneTransform2 buffer allocation ──────────────────────────────────────
    //
    // Field entity[+0x114] es BoneTransform2: array de matrices 3×4 (48 B=0x30)
    // por bone.  FUN_00440060 (Sprite_Draw) itera hasta model[+0x22] (boneCount)
    // y escribe cada bone en buf[boneIdx*0x30]. El buffer necesita al menos
    // `boneCount * 0x30` bytes.
    //
    // ASM original (0045bb70..0045bb8e):
    //   MOV EAX,[0x05828d58]                       ; Models base ptr
    //   MOVSX EAX, word ptr [EAX + type*0xbc + 0x22] ; bone count
    //   LEA ECX,[EAX + EAX*2]                      ; count*3
    //   SHL ECX,0x4                                ; count*0x30
    //   PUSH ECX; CALL operator_new
    //
    // BUG-FIX: el port usaba offset +0x1022 (de una mala interpretación del
    // decompile Ghidra que mostraba "Models[0xee8ef].Data + 0x9e"), lo que leía
    // 22 structs MODEL_t adelante y devolvía un short basura. Si ese short era
    // menor que el bone count real → undersized buffer → overflow detectado por
    // PageHeap en FUN_004f9f70 línea 7266 (crash al escribir el último bone).
    // Correcto: offset +0x22 dentro del struct MODEL_t (mismo que usa
    // FUN_00440060 para su loop count).
    if (*(unsigned char **)(param_1 + 0x114) != NULL) {
        operator_delete(*(unsigned char **)(param_1 + 0x114));
        *(int *)(param_1 + 0x114) = 0;
    }
    {
        short etype     = *(short *)(param_1 + 2);
        short boneCount = *(short *)(DAT_05828d58 + etype * 0xbc + 0x22);
        if (boneCount < 0) boneCount = 0;   // defense: short garbage → signed neg
        void *boneBuf = operator_new((unsigned int)boneCount * 0x30);
        // UB heredado del original: IDA 0045ADC0 L710 hace `operator_new` sin
        // inicializar, y el buffer se LEE antes de escribirse. El tick del frame
        // del spawn (MoveCharacterClient -> MoveCharacterVisual, 0x4520C0) entra
        // al switch por ModelID y transforma huesos que Calc_RenderObject todavia
        // no lleno: recien los llena el render, que corre despues.
        //
        // En el binario release eso devuelve paginas frescas del OS (ceros) y el
        // artefacto no se ve. Con el CRT debug el relleno es 0xCDCDCDCD, o sea
        // posiciones de ~-5.6e8: la cadena de 13 joints 1254 de Queen Rainer
        // (ModelID 321) nace con esas coordenadas y dibuja los haces azules que
        // cruzan la pantalla. Cuadra con el sintoma: al entrar por primera vez
        // los mobs salen mal y al alejarse y volver (slot ya con huesos validos)
        // se ven bien.
        //
        // Se inicializa a cero, que es lo que el original obtiene de hecho.
        // Mismo criterio que el fix de los buffers POT de textura (7c1a39d).
        if (boneBuf && boneCount > 0)
            memset(boneBuf, 0, (size_t)boneCount * 0x30);
        *(void **)(param_1 + 0x114) = boneBuf;
    }

    // ── Equip slot init ───────────────────────────────────────────────────────
    // 2 slots at param_1+0x272 (stride 0x18)
    {
        unsigned char *slot = param_1 + 0x272;
        for (int i = 0; i < 2; i++) {
            *(short *)(slot - 2) = (short)0xffff;
            slot[0] = 0;
            slot[1] = 0;
            slot += 0x18;
        }
    }
    // 6 slots at param_1+0x1e2 (stride 0x18)
    {
        unsigned char *slot = param_1 + 0x1e2;
        for (int i = 0; i < 6; i++) {
            *(short *)(slot - 2) = (short)0xffff;
            slot[0] = 0;
            slot[1] = 0;
            slot += 0x18;
        }
    }

    // ── Final fixed values ────────────────────────────────────────────────────
    *(short *)(param_1 + 0x2a0) = (short)0xffff;
    *(short *)(param_1 + 0x2b8) = (short)0xffff;  // char_class = none
    *(short *)(param_1 + 0x2d0) = (short)0xffff;
    *(int   *)(param_1 + 0x330) = -1;              // 0xffffffff
    param_1[0x32c]               = 0;
    *(int   *)(param_1 + 0xd0)  = 0x43480000;      // 200.0f
    *(int   *)(param_1 + 0x334) = 0;
    *(short *)(param_1 + 0x1be) = 0;
    *(int   *)(param_1 + 0x33c) = -1;
    *(int   *)(param_1 + 100)   = -1;
    *(float *)(param_1 + 0x68)  = 1.0f;

    // Weapon attachment bones. IDA 0045ADC0 L744-857 selects these from the
    // parent model; RenderLinkObject then uses Weapon[n].LinkBone.
    switch ((int)entity_type) {
        case 270: case 300: param_1[0x274] = 42; param_1[0x28c] = 33; break;
        case 271:           param_1[0x274] = 19; param_1[0x28c] = 14; break;
        case 273:           param_1[0x274] = 26; param_1[0x28c] = 36; break;
        case 274: case 275: case 280: param_1[0x274] = 41; param_1[0x28c] = 32; break;
        case 278:           param_1[0x274] = 29; param_1[0x28c] = 38; break;
        case 281: case 286: case 299: case 304: case 305: case 323: case 327:
                            param_1[0x274] = 30; param_1[0x28c] = 39; break;
        case 288:           param_1[0x274] = 26; param_1[0x28c] = 35; break;
        case 289:           param_1[0x274] = 31; param_1[0x28c] = 22; break;
        case 291:           param_1[0x274] = 24; param_1[0x28c] = 19; break;
        case 292:           param_1[0x274] = 25; param_1[0x28c] = 16; break;
        case 294:           param_1[0x274] = 39; param_1[0x28c] = 30; break;
        case 296:           param_1[0x274] = 16; param_1[0x28c] = 25; break;
        case 297:           param_1[0x274] = 17; param_1[0x28c] = 28; break;
        case 306:           param_1[0x274] = 52; param_1[0x28c] = 65; break;
        case 310:           param_1[0x274] = 20; param_1[0x28c] = 33; break;
        case 311:           param_1[0x274] = 23; break;
        case 312:           param_1[0x274] = 43; break;
        case 314:           param_1[0x274] = 55; param_1[0x28c] = 70; break;
        case 316:           param_1[0x274] = 39; param_1[0x28c] = 39; break;
        case 317:           param_1[0x274] = 27; param_1[0x28c] = 38; break;
        case 318:           param_1[0x274] = 32; param_1[0x28c] = 43; break;
        case 322:           param_1[0x274] = 36; param_1[0x28c] = 45; break;
        case 325:           param_1[0x274] = 27; param_1[0x28c] = 18; break;
        case 329:           param_1[0x274] = 33; param_1[0x28c] = 20; break;
        case 330:           param_1[0x274] = 1;  param_1[0x28c] = 1;  break;
        default:             param_1[0x274] = 33; param_1[0x28c] = 42; break;
    }
    // NOTE: IDA writes Rotation to entity+0x24 (Angle[2]) via the type-switch
    // block earlier in this function — do NOT write to +0x28 (that's the
    // BoneAngle / AngleVel slot and writing Rotation there breaks facing).
}
