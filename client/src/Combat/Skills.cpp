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
//   +0x301  byte  SkillSuccess        — bit 15 del target del paquete.  NO es
//                   'is_pvp': el server lo pone con `target[0] |= type * 0x80`
//                   donde `type` es el flag de EXITO del skill
//                   (CSkillManager::GCSkillAttackSend).  IDA 0x42BCA0 hace
//                   `Success = TargetKey >> 15; sc->SkillSuccess = Success != 0`
//                   y MU 5.2 (WSclient.cpp L3642) lo escribe igual.
//   +0x303  byte  combo_counter       — incremented on skill_type 0x17
//   +0x2F4  byte  teleport_state      — 2 = teleporting
//   +0x2F5  byte  is_skill_active     — set to 1 on most skill hits (common tail)
//   +0x2F6  short target_id_pvp       — used during teleport
//
// Helper functions identified:
//   FUN_004742b0  = CreateTeleportBegin — canonical mapping; the port's `Entity_WeaponHit`
//                   helper has no verified FUN mapping.
//   SetPlayerBow (nombre real en IDA; el alias
//                   `Entity_ResetToWalk` es inventado del port).  Toma el
//                   PUNTERO a la entidad, no el indice.
//   SetPlayerMagic  = Entity_SelectTarget_Player — set caster's attack target to player entity
//   SetPlayerDie (per Net/SecondPassword.cpp); the port's `Entity_TeleportEnd`
//                   helper has no verified FUN mapping.
//   FUN_004792c0  = CreatePoint (per Entity_LegacyTeleport.cpp); the port's `Entity_TeleportAnim`
//                   helper has no verified FUN mapping.
//   UIChatLogWindow_AddText  = UI_ShowExpGainOverlay — draws +EXP floating text on screen
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
//   0x30 (48)    Greater Life (Swell Life) — efecto 1150/sub1 con owner sobre
//                el objetivo (aura dorada), target[+120]|=0x10, anim=63
//   0x31         Anim 0x40 or 0x41     — depends on World
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
extern "C" void __cdecl DeleteEffect(int Type, DWORD Owner, int iSubType);

// SetPlayerBow @ 0042BC00. It selects the bow/crossbow animation and keeps
// the walking variant when a path is active.
static void SetPlayerBow97k(BYTE* entity)
{
    if (!entity) return;
    const short leftType = *(short*)(entity + 648);
    if ((leftType >= 528 && leftType < 535) || leftType == 545) {
        SetAction((int)(intptr_t)entity,
                     ((entity[444] & 7) != 2 || *(unsigned short*)(entity + 672) == 0xFFFF) ? 46 : 48);
        return;
    }
    const short rightType = *(short*)(entity + 624);
    if ((rightType >= 536 && rightType < 543) || rightType == 544) {
        SetAction((int)(intptr_t)entity,
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
        SetPlayerMagic((int)(intptr_t)caster);
    else
        SetPlayerAttack((int)(intptr_t)caster, 0, 0, 0);
    *(DWORD*)(caster + 264) = 0;
}

// External data
extern BYTE* g_EntityBase;   // DAT_07abf5d0  legacy alias (may be NULL)
extern int   World; // World
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
    // Bit 15 del target = flag de EXITO del skill (ver la nota de +0x301 arriba).
    int skill_ok    = (target_raw >> 15) & 1;

    int caster_idx  = Entity_FindById(caster_id);
    int target_idx  = Entity_FindById(target_id);

    // Trace at the same point as IDA's LABEL_81, before queue state changes.
    if (skill_type == 3 || skill_type == 7) {
        char trace[160];
        wsprintfA(trace, "SKILL19 RX skill=%d casterKey=%d caster=%d targetKey=%d target=%d pvp=%d",
                  skill_type, caster_id, caster_idx, target_id, target_idx, skill_ok);
        DbgLogPublic(trace);
    }

    if (caster_idx >= 400 || target_idx >= 400)
        return; // 0042BCA0 only enters its state/animation path with a resolved target.

    BYTE* caster = ENTITY(caster_idx);

    // IDA 0x42BCA0 L115: `AttackPlayer = Index` (el slot del CASTER), justo
    // tras resolver el target.  Lo lee ReceiveAttackDamage para orientar el
    // destello de bloqueo.
    AttackPlayer = caster_idx;

    // 0042BCA0 LABEL_81: before its per-skill animation switch, the original
    // records the received skill in the character's visual-effect queue.  The
    // per-frame MoveCharacter dispatcher consumes exactly c+770 once c+757
    // reaches its attack-frame limit; without this assignment it only sees
    // stale/zero data and therefore plays the pose without creating the spell
    // effect.
    *(BYTE*) (caster + 770) = (BYTE)skill_type;

    *(short*)(caster + 0x310) = (short)target_idx;
    // 2026-09-04 FIX: aca habia `(BYTE)(is_pvp == 0)`, o sea el valor INVERTIDO.
    // IDA 0x42BCA0 escribe `sc->SkillSuccess = (TargetKey >> 15) != 0`, y el
    // server MuEmu pone ese bit justamente cuando el skill tuvo exito
    // (`pMsg.target[0] = SET_NUMBERHB(idx) | (type * 0x80)`).
    // Consecuencia: los tres consumidores del flag quedaban al reves --
    // el aura de Greater Defense (MoveCharacter case 27 -> 5x joint 266/sub4)
    // no se creaba nunca, ni el buff de Greater Damage (case 28), ni el
    // congelamiento del Ice Arrow (case 0x33) ni el de Lightning (0x37).
    *(BYTE*) (caster + 0x301) = (BYTE)(skill_ok != 0);

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
        *(float*)(caster + 36) = CreateAngle(
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
    case 0x01:  // 1 Poison
    case 0x02:  // 2 Meteorite
    case 0x04:  // 4 Fire Ball
    case 0x07:  // 7 Ice
    case 0x0B:  // 11 Power Wave
    case 0x11:  // 17 Energy Ball
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

    case 0x03:  // 3 Lightning
    {
        // IDA plays sound 60, then interrupts the four hit/recoil actions on
        // the target before entering LABEL_107 (the shared magic path).
        PlayBuffer(60, 0, 0);
        if (target != nullptr) {
            const BYTE targetAction = target[261];
            if (targetAction == 0x8B || targetAction == 0x8C ||
                targetAction == 0x85 || targetAction == 0x87) {
                SetPlayerStop((int)(intptr_t)target);
            }
        }
        AnimateRemoteSkillCaster97k(caster);
        goto common_tail;
    }

    case 0x06:  // 6 Teleport
    {
        // Unlike the other cases, IDA returns immediately after the
        // teleport-begin animation rather than marking the common skill tail.
        CreateTeleportBegin((unsigned int)(uintptr_t)caster);
        return;
    }

    case 0x0F:  // 15 Teleport Party
    {
        if (target != nullptr)
            CreateTeleportBegin((unsigned int)(uintptr_t)target);
        CreateTeleportEnd((unsigned int)(uintptr_t)caster);
        // IDA 0x42BCA0 L198-201: `if (sc == Hero) Teleport = 0;`
        // (Teleport = 0x05826D14 = Teleport).  Faltaba: el flag quedaba
        // armado despues de terminar el teleport propio.
        if (caster == (BYTE*)DAT_07abf5d8)
            Teleport = 0;
        PlayBuffer(102, (DWORD)(uintptr_t)caster, 0);
        return;
    }

    case 0x10:  // 16 Mana Shield
    {
        // Case 0x10 enters LABEL_98 directly: remote magic animation, with
        // no magic-hit sound.  It is deliberately separate from 0x1A..0x24.
        AnimateRemoteSkillCaster97k(caster);
        goto common_tail;
    }

    case 0x1A:  // 26 Heal
    case 0x1B:  // 27 Greater Defense
    case 0x1C:  // 28 Greater Damage
    case 0x1E:  // 30 Summon Goblin
    case 0x1F:  // 31 Summon Stone Golem
    case 0x20:  // 32 Summon Assassin
    case 0x21:  // 33 Summon Elite Yeti
    case 0x22:  // 34 Summon Dark Knight
    case 0x23:  // 35 Summon Bali
    case 0x24:  // 36 Summon Soldier
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

    case 0x13:  // 19 Falling Slash
    {
        SetAction((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(82, 0, 0);
        goto common_tail;
    }

    case 0x14:  // 20 Lunge
    {
        SetAction((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(83, 0, 0);
        goto common_tail;
    }

    case 0x15:  // 21 Uppercut
    {
        SetAction((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(84, 0, 0);
        goto common_tail;
    }

    case 0x16:  // 22 Cyclone
    {
        // ReceiveMagic case 0x16: SetAction(sc, 0x16 + 37), then sound 85.
        SetAction((int)(intptr_t)caster, skill_type + 37);
        PlayBuffer(85, 0, 0);
        goto common_tail;
    }

    case 0x17:  // 23 Slash
    {
        // The alternating action is controlled by the caster-local counter
        // at +771, not by the current map/state.
        SetAction((int)(intptr_t)caster,
                     (caster[771] & 1) ? 41 : skill_type + 37);
        caster[771] += 1;
        PlayBuffer(85, 0, 0);
        goto common_tail;
    }

    case 0x18:  // 24 Triple Shot
    case 0x34:  // 52 Penetration
    {
        SetPlayerBow97k(caster);
        goto common_tail;
    }

    case 0x2B:  // 43 Death Stab
    {
        SetAction((int)(intptr_t)caster, 67);
        if (caster != (BYTE*)DAT_07abf5d8 && *(WORD*)(caster + 2) == 390)
            *(DWORD*)(caster + 264) = 0;
        goto common_tail;
    }

    case 0x2F:  // 47 Impale
    {
        SetAction((int)(intptr_t)caster, 66);
        goto common_tail;
    }

    case 0x30:  // 48 Greater Life (Swell Life)
    {
        if (target != nullptr)
        {
            // Solo un buff NUEVO crea el efecto 1150/sub1; si ya estaba
            // bufado se refresca el flag y se conserva el efecto vivo.
            // Ese efecto es el que spawnea cada tick las particulas 1150/sub4
            // que se cuelgan de los huesos del personaje (MoveParticles).
            if ((*(DWORD*)(target + 120) & 0x10) != 0x10) {
                DeleteEffect(1150, (DWORD)(uintptr_t)target, 1);
                CreateEffect(1150, (float*)(target + 16),
                             (float*)(target + 28), (float*)(target + 232),
                             (float*)1, (float*)target, (float*)-1, nullptr, 0);
            }
            *(DWORD*)(target + 120) |= 0x10;
        }
        SetAction((int)(intptr_t)caster, 63);
        goto common_tail;
    }

    case 0x31:  // 49 Fire Breath
    {
        SetAction((int)(intptr_t)caster,
                     (World == 8 || World == 10) ? 65 : 64);
        goto common_tail;
    }

    case 0x32:  // 50 Monster Area Attack
        goto common_tail;

    case 0x33:  // 51 Ice Arrow
    {
        SetPlayerBow97k(caster);
        if (target != nullptr && caster[769])
        {
            DeleteEffect(190, (DWORD)(uintptr_t)target, 1);
            float angle[3] = {
                *(float*)(target + 28), *(float*)(target + 32), *(float*)(target + 36)
            };
            CreateEffect(190, (float*)(target + 16), angle, (float*)(target + 232),
                         (float*)1, (float*)target, (float*)-1, nullptr, 0);
            angle[2] += 180.0f;
            CreateEffect(190, (float*)(target + 16), angle, (float*)(target + 232),
                         (float*)2, (float*)target, (float*)-1, nullptr, 0);
            // Clear freeze offset (0x2BC byte at local +0xBB from struct base = 0xBB)
            target[748] = 0;
            SetPlayerStop((int)(intptr_t)target);
            // Set ice status bit
            *(DWORD*)(target + 120) |= 0x20;
        }
        goto common_tail;
    }

    case 0x37:  // 55 Fire Slash
    {
        if (target != nullptr && caster[769])
        {
            DeleteEffect(1274, (DWORD)(uintptr_t)target, 0);
            float light[3] = { 1.0f, 1.0f, 1.0f };
            CreateEffect(1274, (float*)(target + 16), (float*)(target + 28), light,
                         nullptr, (float*)target, (float*)-1, nullptr, 0);
            // Set lightning status bit
            PlayBuffer(104, (DWORD)(uintptr_t)target, 0);
            *(DWORD*)(target + 120) |= 0x40;
        }
        return;
    }

    case 0x38:  // 56 Power Slash
    {
        SetAction((int)(intptr_t)caster, 81);
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
