// Skills.cpp
// PacketHandler_0x19 @ 0x0042bca0  — skill/magic effect dispatch (889 lines)
// PacketHandler_0x16 @ 0x0042db60  — kill confirm + EXP gain (582 lines)
//
// Packet 0x19 format: [C1][len][0x19][skill_type][caster_hi][caster_lo][target_hi][target_lo][damage_hi][damage_lo]
// Packet 0x16 format: [C1][len][0x16][caster_hi][caster_lo][target_hi][target_lo][exp_bytes...]
//
// Entity offsets referenced here (new, not in other files):
//   +0x2EB  byte  tipo de monstruo (Monster.txt).  NO es un "magic_channel_flag":
//                   los unicos writers son CreateCharacterPointer (= -1, o sea 0xFF
//                   para el heroe) y CreateMonster (= Type).  El `!= 77` de mas
//                   abajo compara contra un TIPO, no contra la letra 'M'.
//   +0x301  byte  is_pvp              — target_id >> 15
//   +0x303  byte  combo_counter       — incremented on skill_type 0x17
//   +0x2F4  byte  teleport_state      — 2 = teleporting
//   +0x2F5  byte  is_skill_active     — set to 1 on most skill hits (common tail)
//   +0x2F6  short target_id_pvp       — used during teleport
//
// Helper functions identified:
//   FUN_004742b0  = CreateTeleportBegin — canonical mapping; the port's `Entity_WeaponHit`
//                   helper has no verified FUN mapping.
//   FUN_0042bc00  = SetPlayerBow (nombre real en IDA; el alias
//                   `Entity_ResetToWalk` es inventado del port).  Toma el
//                   PUNTERO a la entidad, no el indice.
//   FUN_00444a80  = Entity_SelectTarget_Player — set caster's attack target to player entity
//   FUN_00444d90  = SetPlayerDie (per Net/SecondPassword.cpp); the port's `Entity_TeleportEnd`
//                   helper has no verified FUN mapping.
//   FUN_004792c0  = CreatePoint (per Entity_LegacyTeleport.cpp); the port's `Entity_TeleportAnim`
//                   helper has no verified FUN mapping.
//   FUN_00480620  = UI_ShowExpGainOverlay — draws +EXP floating text on screen
//   FUN_00474310  = CreateTeleportEnd (per Entity_LegacyTeleport.cpp); the port's
//                   `Entity_MeleeAttackStart` helper has no verified FUN mapping.
//
// Skill type table (byte[3] in packet 0x19):
//   0x01 / 0x11  Arrow / basic ranged  — UI 0x3C, reset walk if mid-attack, select target
//   0x03         Same as 0x01 with extra anim reset check
//   0x06         Weapon sparkle only   — Entity_WeaponHit(caster)
//   0x0F         Melee + ranged hit    — Entity_WeaponHit(caster) + Entity_MeleeAttackStart(caster); UI 0x66
//   0x10         Magic (generic)       — if magic_channel_flag!='M': UI 0x51, Entity_SelectTarget_Player
//   0x13         Skill anim 0x38       — UI 0x52
//   0x14         Skill anim 0x39       — UI 0x53
//   0x15         Skill anim 0x3A       — UI 0x54
//   0x17         Combo hit             — entity[+0x303]++, anim=0x3C or 0x29, UI 0x55
//   0x18 / 0x34  Cancel / reset        — Entity_ResetToWalk(caster)
//   0x1A..0x24   Magic variants        — same as 0x10 (magic_channel_flag check)
//   0x2B         Skill anim 0x43
//   0x30         Poison buff on target — Particle 0x47E, target[+0x1E]|=0x10, anim=0x3F
//   0x31         Anim 0x40 or 0x41     — depends on g_GameSubState
//   0x33         Ice buff on target    — Particle 0xBE×2, target[+0x2BC_byte=0xBB]=0, target[+0x1E]|=0x20
//   0x37         Lightning buff        — Particle 0x4FA, UI 0x68, target[+0x1E]|=0x40
//   0x38         Skill anim 0x51       — UI 0x55
//
// Common tail (most cases jump here before return):
//   entity_caster[+0x2F5] = 1   // mark skill as active/landed

#include "stdafx.h"
#include "../Combat/Skills.h"

extern "C" void DbgLogPublic(const char* msg);

// Forward declarations for functions identified in decompilation
extern "C" void __cdecl CreateTeleportBegin(unsigned int entity); // 0x004742b0
extern "C" void __cdecl CreateTeleportEnd(unsigned int entity);   // 0x00474310
extern void Entity_ResetToWalk(int entity_ptr);             // 0x0042bc00 = SetPlayerBow — recibe PUNTERO
extern void Entity_TeleportEnd(int entity_idx);             // Legacy helper; no verified FUN mapping.
extern void Entity_TeleportAnim(float* world_pos, float, float, float); // Legacy helper; no verified FUN mapping.
extern void UI_ShowExpGainOverlay(int amount);              // 0x00480620
extern void Entity_MeleeAttackStart(int entity_idx);        // Legacy helper; no verified FUN mapping.
extern int  Entity_FindById(int id);                        // 0x0045ac80 (Monster.cpp)
extern void Particle_Spawn(int type, float x, float y, float z, int flags); // particle system
extern "C" void __cdecl DeleteEffect(int Type, DWORD Owner, int iSubType);

// SetPlayerBow @ 0042BC00. It selects the bow/crossbow animation and keeps
// the walking variant when a path is active.
static void SetPlayerBow97k(BYTE* entity)
{
    if (!entity) return;
    const short leftType = *(short*)(entity + 648);
    if ((leftType >= 528 && leftType < 535) || leftType == 545) {
        FUN_0043e820((int)(intptr_t)entity,
                     ((entity[444] & 7) != 2 || *(unsigned short*)(entity + 672) == 0xFFFF) ? 46 : 48);
        return;
    }
    const short rightType = *(short*)(entity + 624);
    if ((rightType >= 536 && rightType < 543) || rightType == 544) {
        FUN_0043e820((int)(intptr_t)entity,
                     ((entity[444] & 7) != 2 || *(unsigned short*)(entity + 672) == 0xFFFF) ? 47 : 49);
    }
}

// Skills_PacketHandler @ 0042BCA0 LABEL_107/LABEL_98.  A broadcast only
// animates its remote caster; it does not invoke a target-selection helper and
// it never replays the local hero's cast animation.
static void AnimateRemoteSkillCaster97k(BYTE* caster)
{
    if (!caster || caster == (BYTE*)DAT_07abf5d8)
        return;
    if (*(WORD*)(caster + 2) == 390)
        FUN_00444a80((int)(intptr_t)caster);
    else
        FUN_00444410((int)(intptr_t)caster, 0, 0, 0);
    *(DWORD*)(caster + 264) = 0;
}

// External data
extern BYTE* g_EntityBase;   // DAT_07abf5d0  legacy alias (may be NULL)
extern int   g_GameSubState; // DAT_0055a7ac
extern BYTE* g_CharData;     // DAT_07cf1ffc

// 2026-05-07: g_EntityBase is never wired to the actual entity array — the real
// base lives in DAT_07abf5d0 (set by WinMain).
#define ENTITY(idx)  ((BYTE*)DAT_07abf5d0 + (idx) * 0x394)
#define STRIDE       0x394

// ============================================================
// PacketHandler_0x19  @ 0x0042bca0
// Server → Client: skill/magic effect
// Packet: [C1][len][19][skill_type][caster_hi][caster_lo][target_hi][target_lo][dmg_hi][dmg_lo]
// ============================================================
// NOTE: Lines 0–360 are the standard XOR handshake / ACK boilerplate shared with
//       0x1E and 0x12 — identical pattern, omitted here (see Net_Process.cpp).
//       Real logic begins at decompile offset ~360.
void PacketHandler_0x19(BYTE* pkt)
{
    // --- XOR ACK block (lines 0-360) omitted — see Net_Process.cpp boilerplate ---

    // Decode caster and target IDs
    int caster_raw  = (pkt[4] << 8) | pkt[5];
    int target_raw  = (pkt[6] << 8) | pkt[7];
    int skill_type  = pkt[3];

    int caster_id   = caster_raw & 0x7FFF;
    int target_id   = target_raw & 0x7FFF;
    int is_pvp      = (target_raw >> 15) & 1;

    int caster_idx  = Entity_FindById(caster_id);
    int target_idx  = Entity_FindById(target_id);

    // Trace at the same point as IDA's LABEL_81, before queue state changes.
    if (skill_type == 3 || skill_type == 7) {
        char trace[160];
        wsprintfA(trace, "SKILL19 RX skill=%d casterKey=%d caster=%d targetKey=%d target=%d pvp=%d",
                  skill_type, caster_id, caster_idx, target_id, target_idx, is_pvp);
        DbgLogPublic(trace);
    }

    if (caster_idx >= 400 || target_idx >= 400)
        return; // 0042BCA0 only enters its state/animation path with a resolved target.

    BYTE* caster = ENTITY(caster_idx);

    // 0042BCA0 LABEL_81: before its per-skill animation switch, the original
    // records the received skill in the character's visual-effect queue.  The
    // per-frame MoveCharacter dispatcher consumes exactly c+770 once c+757
    // reaches its attack-frame limit; without this assignment it only sees
    // stale/zero data and therefore plays the pose without creating the spell
    // effect.
    *(BYTE*) (caster + 770) = (BYTE)skill_type;

    // 0042BCA0 writes caster+769 as the inverse of the target PvP bit.
    // Cases 0x33/0x37 use this as their normal/PvE-target gate.
    *(short*)(caster + 0x310) = (short)target_idx;
    *(BYTE*) (caster + 0x301) = (BYTE)(is_pvp == 0);

    if (skill_type == 3 || skill_type == 7) {
        char trace[128];
        wsprintfA(trace, "SKILL19 QUEUE skill=%d caster=%d targetSlot=%d stage=%u",
                  skill_type, caster_idx, target_idx, (unsigned)caster[757]);
        DbgLogPublic(trace);
    }

    BYTE* target = (target_idx < 400) ? ENTITY(target_idx) : nullptr;

    // 0042BCA0 turns a remote caster toward its visible target before the
    // per-skill animation is selected.  The hero is intentionally excluded:
    // its facing is driven by local input/Attack(), not by the echoed packet.
    if (caster != (BYTE*)DAT_07abf5d8 && target != nullptr &&
        skill_type != 6 && skill_type != 15 && target[352] != 0) {
        *(float*)(caster + 36) = FUN_0043e050(
            *(float*)(caster + 16), *(float*)(caster + 20),
            *(float*)(target + 16), *(float*)(target + 20));
    }

    // -------------------------------------------------------
    // Skill type dispatch
    // -------------------------------------------------------
    switch (skill_type)
    {
    // IDA LABEL_107: these six projectile/magic IDs share the exact
    // SetPlayerMagic/SetPlayerAttack path before the common active tail.
    case 0x01:  // Poison
    case 0x02:  // Meteorite
    case 0x04:  // Fire Ball
    case 0x07:  // Ice
    case 0x0B:  // Power Wave
    case 0x11:  // Energy Ball
    {
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
        AnimateRemoteSkillCaster97k(caster);
        goto common_tail;
    }

    case 0x03:  // Ranged with extra anim reset
    {
        // IDA plays sound 60, then interrupts the four hit/recoil actions on
        // the target before entering LABEL_107 (the shared magic path).
        PlayBuffer(60, 0, 0);
        if (target != nullptr) {
            const BYTE targetAction = target[261];
            if (targetAction == 0x8B || targetAction == 0x8C ||
                targetAction == 0x85 || targetAction == 0x87) {
                FUN_004430c0((int)(intptr_t)target);
            }
        }
        AnimateRemoteSkillCaster97k(caster);
        goto common_tail;
    }

    case 0x06:  // Teleport
    {
        // Unlike the other cases, IDA returns immediately after the
        // teleport-begin animation rather than marking the common skill tail.
        CreateTeleportBegin((unsigned int)(uintptr_t)caster);
        return;
    }

    case 0x0F:  // Melee + ranged combined hit
    {
        if (target != nullptr)
            CreateTeleportBegin((unsigned int)(uintptr_t)target);
        CreateTeleportEnd((unsigned int)(uintptr_t)caster);
        // FUN_00413900(0x66, caster_idx) — UI event 0x66 = dual hit
        PlayBuffer(102, (DWORD)(uintptr_t)caster, 0);
        return;
    }

    case 0x10:
    {
        // Case 0x10 enters LABEL_98 directly: remote magic animation, with
        // no magic-hit sound.  It is deliberately separate from 0x1A..0x24.
        AnimateRemoteSkillCaster97k(caster);
        goto common_tail;
    }

    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1E:
    case 0x1F:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    {
        // 0042BCA0 first plays 81 unless the channel state is 77, then
        // enters LABEL_98 for the remote caster animation.
        if (caster[747] != 77)
            PlayBuffer(81, 0, 0);

        // LABEL_98 applies this animation to remote casters only.
        if (caster != (BYTE*)DAT_07abf5d8)
        {
            // FUN_00413900(0x51, caster_idx) — UI event 0x51 = magic hit
            AnimateRemoteSkillCaster97k(caster);
        }
        goto common_tail;
    }

    case 0x13:  // Skill anim 0x38 (class-specific)
    {
        FUN_0043e820((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(82, 0, 0);
        goto common_tail;
    }

    case 0x14:  // Skill anim 0x39
    {
        FUN_0043e820((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(83, 0, 0);
        goto common_tail;
    }

    case 0x15:  // Skill anim 0x3A
    {
        FUN_0043e820((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(84, 0, 0);
        goto common_tail;
    }

    case 0x16:  // Mana Shield
    {
        // ReceiveMagic case 0x16: SetAction(sc, 0x16 + 37), then sound 85.
        FUN_0043e820((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(85, 0, 0);
        goto common_tail;
    }

    case 0x17:  // Combo hit
    {
        // The alternating action is controlled by the caster-local counter
        // at +771, not by the current map/state.
        FUN_0043e820((int)(intptr_t)caster,
                     (caster[771] & 1) ? 41 : skill_type + 37);
        caster[771] += 1;
        PlayBuffer(85, 0, 0);
        goto common_tail;
    }

    case 0x18:  // Triple Shot
    case 0x34:  // Penetration
    {
        SetPlayerBow97k(caster);
        goto common_tail;
    }

    case 0x2B:  // Skill anim 0x43
    {
        FUN_0043e820((int)(intptr_t)caster, 67);
        if (caster != (BYTE*)DAT_07abf5d8 && *(WORD*)(caster + 2) == 390)
            *(DWORD*)(caster + 264) = 0;
        goto common_tail;
    }

    case 0x2F:  // Death Stab
    {
        FUN_0043e820((int)(intptr_t)caster, 66);
        goto common_tail;
    }

    case 0x30:  // Poison buff on target
    {
        if (target != nullptr)
        {
            // Original first checks the status mask.  Only a new poison
            // builds effect 1150; a refreshed one keeps the existing effect.
            if ((*(DWORD*)(target + 120) & 0x10) != 0x10) {
                DeleteEffect(1150, (DWORD)(uintptr_t)target, 1);
                FUN_00460dc0(1150, (float*)(target + 16),
                             (float*)(target + 28), (float*)(target + 232),
                             (float*)1, (float*)target, (float*)-1, nullptr, 0);
            }
            *(DWORD*)(target + 120) |= 0x10;
        }
        FUN_0043e820((int)(intptr_t)caster, 63);
        goto common_tail;
    }

    case 0x31:  // Anim by game state
    {
        FUN_0043e820((int)(intptr_t)caster,
                     (World == 8 || World == 10) ? 65 : 64);
        goto common_tail;
    }

    case 0x32:
        goto common_tail;

    case 0x33:  // Ice Arrow
    {
        SetPlayerBow97k(caster);
        if (target != nullptr && caster[769])
        {
            DeleteEffect(190, (DWORD)(uintptr_t)target, 1);
            float angle[3] = {
                *(float*)(target + 28), *(float*)(target + 32), *(float*)(target + 36)
            };
            FUN_00460dc0(190, (float*)(target + 16), angle, (float*)(target + 232),
                         (float*)1, (float*)target, (float*)-1, nullptr, 0);
            angle[2] += 180.0f;
            FUN_00460dc0(190, (float*)(target + 16), angle, (float*)(target + 232),
                         (float*)2, (float*)target, (float*)-1, nullptr, 0);
            // Clear freeze offset (0x2BC byte at local +0xBB from struct base = 0xBB)
            target[748] = 0;
            FUN_004430c0((int)(intptr_t)target);
            // Set ice status bit
            *(DWORD*)(target + 120) |= 0x20;
        }
        goto common_tail;
    }

    case 0x37:  // Lightning
    {
        if (target != nullptr && caster[769])
        {
            DeleteEffect(1274, (DWORD)(uintptr_t)target, 0);
            float light[3] = { 1.0f, 1.0f, 1.0f };
            FUN_00460dc0(1274, (float*)(target + 16), (float*)(target + 28), light,
                         nullptr, (float*)target, (float*)-1, nullptr, 0);
            // Set lightning status bit
            PlayBuffer(104, (DWORD)(uintptr_t)target, 0);
            *(DWORD*)(target + 120) |= 0x40;
        }
        return;
    }

    case 0x38:  // Power Slash
    {
        FUN_0043e820((int)(intptr_t)caster, 81);
        PlayBuffer(85, 0, 0);
        goto common_tail;
    }

    default:
        return;
    }

common_tail:
    // Common tail: mark skill as active on caster
    // Almost all skill_type cases jump here
    *(BYTE*)(caster + 0x2F5) = 1;
}


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
//   FUN_00480620(exp_gained)  → floating "+EXP" overlay
// ============================================================
void PacketHandler_0x16(BYTE* pkt)
{
    // --- XOR ACK block (lines 0-480) omitted ---

    int caster_raw = (pkt[3] << 8) | pkt[4];
    int target_raw = (pkt[5] << 8) | pkt[6];

    int caster_id  = caster_raw & 0x7FFF;
    int target_id  = target_raw & 0x7FFF;
    int teleport_start = !(pkt[3] & 0x80);  // 0=TeleportEnd, 1=TeleportStart

    int caster_idx = Entity_FindById(caster_id);
    int target_idx = Entity_FindById(target_id);

    if (caster_idx >= 400)
        return;

    BYTE* caster = ENTITY(caster_idx);

    // Orange color constant used for kill flash: {1.0f, 0.6f, 0.0f}
    // (stored in FPU literals inside the function, referenced for color override)

    if (teleport_start)
    {
        // Begin teleport animation
        // entity[+0x2F4] = 2 → teleporting state
        *(BYTE*)(caster + 0x2F4) = 2;
        *(short*)(caster + 0x2F6) = (short)target_id;
        *(short*)(caster + 0x310) = (short)target_idx;

        if (target_idx < 400)
        {
            BYTE* target_ent = ENTITY(target_idx);
            float* target_pos = (float*)(target_ent + 0x10);
            // FUN_004792c0(target_pos, ?, ?, ?) — TeleportAnimation at target world pos
            Entity_TeleportAnim(target_pos, 0.0f, 0.0f, 0.0f);
        }
    }
    else
    {
        // Teleport end — snap entity to destination
        Entity_TeleportEnd(caster_idx);
    }

    // Mark target as dead (kill confirm)
    if (target_idx < 400)
    {
        BYTE* target = ENTITY(target_idx);
        *(BYTE*)(target + 0x2FD) = 1;  // is_dead = 1
        *(BYTE*)(target + 0x2EC) = 0;  // clear some state
    }

    // -------------------------------------------------------
    // EXP gain decode (for local player kill confirm)
    // g_CharData XOR-decoding block produces iStack_d94 = exp_gained
    // -------------------------------------------------------
    // The function re-encodes g_CharData with the same 32-byte XOR key (see WinMain.cpp)
    // then adds the decoded EXP:
    //
    //   int exp_gained = <decoded from pkt[7..10]>;
    //   *(int*)(g_CharData + 0x10) += exp_gained;    // total EXP counter
    //
    // If exp_gained > 0, show floating text overlay:
    //   if (local_d90 > 0)
    //       UI_ShowExpGainOverlay(exp_gained);        // FUN_00480620

    // (Exact XOR decode of pkt[7..10] uses the same 32-byte key as login packet —
    //  see WinMain.cpp Net_Connect for key bytes {0xe7,0x6d,0x3a,...,0xe8,0x56})
}
