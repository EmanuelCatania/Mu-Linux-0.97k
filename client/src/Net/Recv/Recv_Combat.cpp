// Recv_Combat.cpp — paquetes del server: combate: daño, acciones, skills y muertes.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Game/HeroVitals.h"
#include "Net/Recv/NetRecv.h"

void ApplyPersistentSkillEffect97k(BYTE* entity, WORD effect, BYTE state)
{
    if (!entity || effect == 0) return;
    DWORD& physicalEffects = *(DWORD*)(entity + 120);

    if (state == 1) {
        if ((effect & 0x10) != 0) {
            DeleteEffect(1150, (DWORD)(uintptr_t)entity, 1);
            CreateEffect(1150, (float*)(entity + 16), (float*)(entity + 28),
                         (float*)(entity + 232), (float*)1, (float*)entity,
                         (float*)-1, nullptr, 0);
        }
        if ((effect & 0x20) != 0 && (physicalEffects & 0x20) == 0) {
            DeleteEffect(190, (DWORD)(uintptr_t)entity, 1);
            float angle[3] = { *(float*)(entity + 28), *(float*)(entity + 32),
                               *(float*)(entity + 36) };
            CreateEffect(190, (float*)(entity + 16), angle, (float*)(entity + 232),
                         (float*)1, (float*)entity, (float*)-1, nullptr, 0);
            angle[2] += 180.0f;
            CreateEffect(190, (float*)(entity + 16), angle, (float*)(entity + 232),
                         (float*)2, (float*)entity, (float*)-1, nullptr, 0);
        }
        if ((effect & 0x40) != 0 && (physicalEffects & 0x40) == 0) {
            DeleteEffect(1274, (DWORD)(uintptr_t)entity, 0);
            float light[3] = { 1.0f, 1.0f, 1.0f };
            CreateEffect(1274, (float*)(entity + 16), (float*)(entity + 28),
                         light, nullptr, (float*)entity, (float*)-1, nullptr, 0);
            PlayBuffer(104, (DWORD)(uintptr_t)entity, 0);
        }
        if ((effect & 0x80) != 0 && (physicalEffects & 0x80) == 0) {
            DeleteEffect(1274, (DWORD)(uintptr_t)entity, 3);
            float light[3] = { 1.0f, 1.0f, 1.0f };
            CreateEffect(1274, (float*)(entity + 16), (float*)(entity + 28),
                         light, (float*)3, (float*)entity, (float*)-1, nullptr, 0);
        }
        if ((effect & 0x100) != 0 && (physicalEffects & 0x100) == 0 &&
            *(WORD*)(entity + 2) != 325) {
            PlayBuffer(103, 0, 0);
            DeleteJoint(266, (DWORD)(uintptr_t)entity, 0);
            for (int i = 0; i < 5; ++i) {
                Joint_Create(266, (float*)(entity + 16), (float*)(entity + 16),
                              (float*)(entity + 28), 0,
                              (int)(uintptr_t)entity, 50.0f, -1, 0);
            }
        }
        physicalEffects |= effect;
        return;
    }

    switch (effect) {
    case 0x08:  DeleteJoint(266, (DWORD)(uintptr_t)entity, 4); break;
    case 0x10:  DeleteEffect(1150, (DWORD)(uintptr_t)entity, 1); break;
    case 0x40:  DeleteEffect(1274, (DWORD)(uintptr_t)entity, 0); break;
    case 0x80:  DeleteEffect(1274, (DWORD)(uintptr_t)entity, 3); break;
    case 0x100: DeleteJoint(266, (DWORD)(uintptr_t)entity, 0); break;
    default: break;
    }
    physicalEffects &= ~((DWORD)effect);
}

// CreateMagicShiny @ 004741E0. ReceiveMagicPosition lo llama en la
// primera mano del caster antes de la animación del hechizo. El pase de render de entidades
// keeps the model and bone matrices at +276, so no synthetic screen-space
// acá no hace falta posicionarlo.
void CreateMagicShiny97k(BYTE* entity, int hand)
{
    if (!entity || hand < 0 || hand > 1 || DAT_05828d58 == 0) return;
    const DWORD bones = *(DWORD*)(entity + 276);
    if (bones == 0) return;

    const WORD type = *(WORD*)(entity + 2);
    const BYTE bone = entity[628 + hand * 24];
    float offset[3] = { 0.0f, 0.0f, 0.0f };
    float position[3];
    float light[3] = { 1.0f, 0.5f, 0.2f };
    void* const model = (void*)(uintptr_t)(DAT_05828d58 + type * 0xBC);

    BMD_TransformPosition(model, (float*)(uintptr_t)(bones + bone * 0x30),
                 offset, position, 1);
    // CreateSprite(1231, Position, 1.0, Light, Hand, 0.0, Character).
    // El quinto argumento es el owner y el último es el subtipo; esto
    // preserva el orden exacto de parámetros de 004741E0_CreateMagicShiny.c.
    CreateSprite(1231, position, 1.0f, light, hand, 0.0f, (int)(uintptr_t)entity);
    CreateSprite(1231, position, 1.0f, light, hand + 2, 0.0f, (int)(uintptr_t)entity);
}

// 0x15
// DESVIACION (DLL Protocol.cpp GCDamageRecv): con GAMESERVER_EXTRA la vida
// que le queda al héroe viaja en ViewCurHP (+8); manda sobre la resta local.
static void SyncHeroLife(const BYTE* Msg, int Size, WORD* life)
{
    if (Size < 16) return;
    const DWORD view = *(const DWORD*)(Msg + 8);
    *life = view > 0xFFFF ? 0xFFFF : (WORD)view;
    gHeroVitals.SetCurrent(VITAL_LIFE, view);
}

void NetRecv_15(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Opcode 0x15 server→cliente = GCDamageSend → PMSG_DAMAGE_SEND per
    // Mu-linux-97K/Source/MuServer/GameServer/Protocol.cpp:1595-1626.
    //
    // Layout (basic, 7 bytes; con GAMESERVER_EXTRA es 15):
    //   [C1][07][0x15]
    //   [3] = (target_idx_hi & 0x7F) | (kill_flag << 7)
    //   [4] = target_idx_lo
    //   [5] = (damage_hi & 0x0F) | (damage_type & 0xF0)
    //   [6] = damage_lo
    //
    // El kill_flag (bit 7 de Msg[3]) es CRÍTICO: cuando vale 1, este
    // hit MATÓ al target. El cliente debe entonces:
    //   - Setear dead_flag (+0x34e = 1) para que no sea targetable
    //   - Disparar animación de muerte
    //   - Mostrar "+EXP / -damage" en HUD (placeholder por ahora)
    //
    // Port FIEL desde IDA ReceiveAttackDamage (0x0042ACC0). Parses damage + color flags,
    // le baja HP al héroe si el objetivo es el héroe, y llama a CreatePoint
    // (FUN_004792c0) para spawnear los números de daño flotantes en el espacio del mundo.
    if (Size < 7) {
        NetLog("NET:  → 0x15 Damage size=%d (too small, skip)", Size);
        return;
    }
    // Wire format: index[0] bits 0-6 + index[1] = id; el BIT ALTO es el
    // flag de ATURDIMIENTO, no un "kill flag" (la etiqueta vieja mentia).
    // Server (Protocol.cpp GCDamageSend):
    //     index[0] = (SET_NUMBERHB(bIndex) & 0x7F) | ((flag & 1) << 7)
    // y `flag` sale de la tirada de stun de Attack.cpp L441:
    //     if (rand() % 100 < m_DamageStuckRate[targetClass]) flag = 1;
    // cancelada si el objetivo va montado en Uniria/Dinorant y la config
    // lo prohibe.  Es lo que hace retroceder al que recibe el Lightning.
    BYTE  stunFlag  = (Msg[3] >> 7) & 0x01;
    WORD  targetId  = ((WORD)(Msg[3] & 0x7F) << 8) | Msg[4];
    // Damage: lower 12 bits across damage[0..1]; upper 4 bits of damage[0]
    // are damage-type flags.
    DWORD damage    = ((DWORD)(Msg[5] & 0x0F) << 8) | Msg[6];
    // El campo legacy `damage[2]` sólo lleva 12 BITS: el server hace
    //     damage[0] = (SET_NUMBERHB(dmg) & 0x0F) | (type & 0xF0);
    //     damage[1] =  SET_NUMBERLB(dmg);
    // o sea el nibble ALTO de damage[0] es el tipo y sólo quedan 4
    // bits para el byte alto del daño → máximo 0x0FFF = 4095, y por
    // encima de eso se pierden los bits 12+.
    //
    // Con `GAMESERVER_EXTRA=1` (nuestro server: el paquete llega con
    // size=16) el valor REAL viaja sin truncar en `ViewDamageHP`.
    // Layout de PMSG_DAMAGE_SEND (Protocol.h:203), con el padding
    // que mete el DWORD:
    //     +0..2  header      +3,+4  index[2]
    //     +5,+6  damage[2]   +7     PADDING
    //     +8..11 ViewCurHP   +12..15 ViewDamageHP
    if (Size >= 16) {
        const DWORD viewDamage = *(const DWORD*)(Msg + 12);
        if (viewDamage != 0) damage = viewDamage;
    }
    BYTE  typeBits  = (Msg[5] >> 4) & 0x0F;
    bool  bIgnore    = (typeBits & 1) != 0;  // yellow (PvP defense ignore)
    bool  bReflect   = (typeBits & 2) != 0;  // purple (reflect dmg)
    bool  bExcellent = (typeBits & 4) != 0;  // cyan
    bool  bCritical  = (typeBits & 8) != 0;  // blue
    NetLog("NET:  → 0x15 Damage tgt=%d dmg=%d flags=I%d/R%d/E%d/C%d kill=%d",
           targetId, damage, bIgnore, bReflect, bExcellent, bCritical, stunFlag);

    // Find target entity slot by entity_id (+0x1dc)
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    BYTE* tgtSlot = nullptr;
    if (targetId == g_HeroKey && DAT_07abf5d8) {
        tgtSlot = (BYTE*)DAT_07abf5d8;
    } else {
        for (int s = 0; s < 400; ++s) {
            BYTE* sp = basePtr + s * 0x394;
            if (sp[0] && *(WORD*)(sp + 0x1dc) == targetId) {
                tgtSlot = sp; break;
            }
        }
    }
    if (!tgtSlot) {
        NetLog("NET:    0x15 SKIP - target id=%d not found", targetId);
        return;
    }

    // ── Golpe con ATURDIMIENTO (bit alto del index) ──────────────
    // IDA ReceiveAttackDamage: TODO el cuerpo de abajo vive dentro de
    // `if (!(Key >> 15))`, y cuando el bit SI esta puesto la funcion
    // toma una rama corta y propia (L263-271):
    //     SetPlayerShock(c, Damage);          // sin rand y sin filtrar
    //     CreatePoint(c+16, Damage, rojo, 15.0);
    //     if (Key == HeroKey) { CharacterAttribute+28 -= Damage;
    //                           c+760 = Damage; }
    //
    // Ese SetPlayerShock es OTRO call site (0x42AD66) que el DLL de
    // inyeccion NO hookea -- solo suprime el de 0x42B33D, el de la
    // tirada 50/50 de mas abajo.  O sea aca el heroe SI se aturde, y
    // por eso en el cliente de referencia el Lightning te frena.
    if (stunFlag) {
        extern void __cdecl SetPlayerShock(int c, int Hit);
        SetPlayerShock((int)tgtSlot, (int)damage);
        float pos[3] = { *(float*)(tgtSlot + 0x10),
                         *(float*)(tgtSlot + 0x14),
                         *(float*)(tgtSlot + 0x18) };
        float color[3] = { 1.0f, 0.0f, 0.0f };
        CreatePoint(pos, (int)damage, color, 15.0f);
        if (targetId == g_HeroKey && DAT_07cf1ff4) {
            WORD* pHP = (WORD*)((BYTE*)(uintptr_t)DAT_07cf1ff4 + 28);
            if (damage < *pHP) *pHP -= damage; else *pHP = 0;
            *(WORD*)(tgtSlot + 760) = (WORD)damage;
            SyncHeroLife(Msg, Size, pHP);
        }
        return;
    }

    // ── Hero HP decrement ────────────────────────────────────────
    // CharacterAttribute del héroe en DAT_07cf1ff4; HP en el offset 28 (WORD).
    if (targetId == g_HeroKey && DAT_07cf1ff4) {
        WORD* pHP = (WORD*)((BYTE*)(uintptr_t)DAT_07cf1ff4 + 28);
        if (damage < *pHP) *pHP -= damage;
        else *pHP = 0;
        SyncHeroLife(Msg, Size, pHP);
    }

    // ── Destello de bloqueo (efecto 259) ────────────────────────
    // IDA ReceiveAttackDamage L171-182, dentro de la rama
    // `Key == HeroKey`.  Sale solo con el buff 0x100 activo
    // (`c+120`, el bitfield que llena InsertBuffPhysicalEffect) y
    // solo si el heroe esta MIRANDO al atacante: el angulo hacia el
    // agresor tiene que estar a menos de 10 grados del facing.
    if (targetId == g_HeroKey &&
        (*(DWORD*)(tgtSlot + 120) & 0x100) == 0x100 &&
        basePtr && AttackPlayer >= 0 && AttackPlayer < 400)
    {
        const float* cm = (const float*)(basePtr + 916 * AttackPlayer);
        const float fAngle = CreateAngle(cm[4], cm[5],
                                          *(float*)(tgtSlot + 16),
                                          *(float*)(tgtSlot + 20));
        if (fabsf(fAngle - cm[9]) < 10.0f) {
            float ang[3] = { 0.0f, 0.0f, fAngle + 180.0f };
            CreateEffect(259, (float*)(tgtSlot + 16), ang,
                         (float*)(tgtSlot + 232),
                         (float*)0, (float*)tgtSlot,
                         (float*)(intptr_t)-1, (float*)0, 0);
        }
    }

    // ReceiveAttackDamage (IDA 0042ACC0) no transiciona una
    // entidad a muerta y nunca llama a SetPlayerDie. El bit alto
    // identifica el golpe terminal para presentar el daño, pero
    // la transición de estado llega en ReceiveDie (0x17). Llamar a
    // SetPlayerDie acá hacía que el héroe/objetivo local entrara en su
    // action before MoveCharacter's native death sequence.

    // ── Cachea el último daño en la entidad (+0x2F8 = 760) per IDA ──
    *(WORD*)(tgtSlot + 760) = damage;

    // ── Hit reaction (anim + grunt) — SetPlayerShock ─────────────
    // Imported from companion-DLL `IgnoreRandomStuck`
    // patch (Patchs.cpp). The original 0.97k client rolls a 50/50
    // el chequeo aleatorio adentro de ReceiveAttackDamage y llama a
    // SetPlayerShock incondicionalmente cuando la tirada pasa — eso
    // produce el bug del "random stuck", donde el héroe se traba en la
    // anim de shock 130 en medio del combate. El DLL companion hookea el
    // call site para saltearlo cuando la entidad es el jugador (tipo 390).
    // Reproducimos el mismo comportamiento acá: los monstruos reciben
    // su anim de shock + el quejido; el jugador NO.
    if (damage > 0 && !stunFlag &&
        *(WORD*)(tgtSlot + 2) != 390)
    {
        // 50/50 random roll matching IDA `rand() & 0x80000001`.
        unsigned r = (unsigned)rand() & 0x80000001u;
        bool fire = (r == 0) ||
                    (((int)r < 0) &&
                     ((((char)r - 1) | (int)0xFFFFFFFE) == -1));
        if (fire) {
            extern void __cdecl SetPlayerShock(int c, int Hit);
            SetPlayerShock((int)tgtSlot, (int)damage);
        }
    }

    // ── Spawn damage popup ───────────────────────────────────────
    // CreatePoint(Position[3], Value, Color[3], scale)
    // Per IDA ReceiveAttackDamage:
    //   damage==0 → MISS: scale=15, color white(hero) or gray(other)
    //   IGNORE   → yellow (1,1,0)  scale=50
    //   EXCELLENT→ cyan   (0,1,0.6)scale=50
    //   CRITICAL → blue   (0,0.6,1)scale=50
    //   REFLECT  → purple (1,0,1)  scale=15
    //   else hero target  → red    (1,0,0)
    //   else other target → orange (1,0.6,0)
    float pos[3];
    pos[0] = *(float*)(tgtSlot + 0x10);
    pos[1] = *(float*)(tgtSlot + 0x14);
    pos[2] = *(float*)(tgtSlot + 0x18);
    float color[3];
    float scale;
    int   displayValue;
    if (damage == 0) {
        displayValue = -1;  // MISS sentinel
        scale = 15.0f;
        if (targetId == g_HeroKey) {
            color[0] = color[1] = color[2] = 1.0f;     // white miss
        } else {
            color[0] = color[1] = color[2] = 0.5f;     // gray miss
        }
    } else {
        displayValue = (int)damage;
        scale = 15.0f;
        if (bIgnore) {
            scale = 50.0f;
            color[0] = 1.0f; color[1] = 1.0f; color[2] = 0.0f;  // yellow
        } else if (bExcellent) {
            scale = 50.0f;
            color[0] = 0.0f; color[1] = 1.0f; color[2] = 0.6f;  // cyan
        } else if (bCritical) {
            scale = 50.0f;
            color[0] = 0.0f; color[1] = 0.6f; color[2] = 1.0f;  // blue
        } else if (bReflect) {
            color[0] = 1.0f; color[1] = 0.0f; color[2] = 1.0f;  // purple
        } else if (targetId == g_HeroKey) {
            color[0] = 1.0f; color[1] = 0.0f; color[2] = 0.0f;  // red
        } else {
            color[0] = 1.0f; color[1] = 0.6f; color[2] = 0.0f;  // orange
        }
    }
    CreatePoint(pos, displayValue, color, scale);
}

// 0x18
void NetRecv_18(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Port FIEL desde IDA ReceiveAction (0x0042B4F0). Server PMSG_ACTION_SEND format:
    //   struct {
    //     PBMSG_HEAD header;  // [C1][size][0x18]
    //     BYTE index[2];      // [3..4] entity index BIG-endian
    //     BYTE dir;           // [5] direction (1..8) → angle = (dir-1)*45
    //     BYTE action;        // [6] action code (NOT raw anim_state)
    //   };
    //
    // Ojo: el action es Msg[6], no el dir de Msg[5], y no se escribe crudo como
    // anim_state.
    //
    // Action codes per IDA (con sufijo "(walk)" si bit 1 de c+444==2):
    //   18  → emote 92 (sound 81)
    //   100 → SetPlayerAttack (atk1) — dispatched por weapon
    //   101 → SetPlayerAttack (atk2)
    //   102/103 → SetPlayerStop (cancel)
    //   108..125 → emotes: pares walk/idle (anim 93..122)
    //   126..131 → special anims 123..127
    //   else → SetAction(c, raw action)
    if (Size < 7) return;
    // IDA ReceiveAction:14 NO maskea el bit 7 de Msg[3]: es el byte alto del Key
    // completo (16 bits).  El `& 0x7F` (kill_flag) es del 0x15, NO del 0x18:
    // aplicarlo acá hace fallar el match con +0x1dc del slot real o anima una
    // entidad equivocada.
    WORD entityKey = (WORD)((Msg[3] << 8) | Msg[4]);
    BYTE dirByte  = Msg[5];                  // [5] = dir (1..8)
    BYTE action   = Msg[6];                  // [6] = action code
    NetLog("NET:  → 0x18 Action key=0x%04x dir=%d act=%d", entityKey, dirByte, action);

    // FindCharacterIndex equivalent: walk CharactersClient[400]
    // matching +0x1dc == entityKey. NO hero-key shortcut — el
    // hero es solo otra entity en el pool. Si no hay match, bail.
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    BYTE* slot = nullptr;
    int slotIdx = -1;
    for (int s = 0; s < 400; ++s) {
        BYTE* sp = basePtr + s * 0x394;
        if (sp[0] && *(WORD*)(sp + 0x1dc) == entityKey) {
            slot = sp; slotIdx = s; break;
        }
    }
    if (!slot) {
        NetLog("NET:    0x18 SKIP key=0x%04x not found", entityKey);
        return;
    }
    // ReceiveAction uses FindCharacterIndex without a Hero
    // excepción: una acción válida para el jugador local tiene que seguir
    // el mismo camino de animación que cualquier otra entidad.
    const bool isHero = (slot == (BYTE*)DAT_07abf5d8);
    NetLog("NET:    0x18 RESOLVED key=0x%04x -> slot=%d isHero=%d type=0x%04x",
           entityKey, slotIdx, isHero ? 1 : 0,
           (int)*(WORD*)(slot + 2));

    // Update facing per IDA ReceiveAction:21:
    //   *(float*)(c + 36) = (dir - 1) * 45.0
    *(float*)(slot + 0x24) = ((float)(int)dirByte - 1.0f) * 45.0f;
    slot[0x2EC] = 0;  // alive flag cleared (per IDA c+748=0)
    // ReceiveAction también aplica la grilla objetivo recibida antes
    // (+774/+775) a la posición de mundo, antes de la animación.
    *(float*)(slot + 16) = (float)slot[774] * 100.0f + 50.0f;
    *(float*)(slot + 20) = (float)slot[775] * 100.0f + 50.0f;

    // Helper: equivalente a SetAction — setea anim_state guardando el previo en cache
    auto SetAnim = [](BYTE* s, BYTE animId) {
        s[0x106] = s[0x105];
        s[0x105] = animId;
        *(float*)(s + 0x108) = 0.0f;
    };
    bool walking = (slot[0x1bc] & 7) == 2;

    switch (action) {
    case 18:
        SetAnim(slot, 92);
        PlayBuffer(81, 0, 0);
        break;
    case 100: case 101: {
        // Ataque despachado — llama a SetPlayerAttack para animar por
        // weapon equipped. SetPlayerAttack is SetPlayerAttack.
        extern void __cdecl SetPlayerAttack(int, int, int, int);
        SetPlayerAttack((int)slot, 0, 0, 0);
        AttackPlayer = slotIdx;      // IDA: AttackPlayer = Index
        slot[0x2F5] = 1;             // c+757=1 attack pending
        *(int*)(slot + 0x108) = 0;   // reset frame
        *(WORD*)(slot + 0x310) = 0xFFFF;  // c+784 = -1 (no skill target)
        break;
    }
    case 102: case 103: {
        extern void __cdecl SetPlayerStop(int);  // SetPlayerStop
        SetPlayerStop((int)slot);
        break;
    }
    case 108: SetAnim(slot, walking ? 135 : 133); break;
    case 109: SetAnim(slot, walking ? 140 : 139); break;
    case 110: SetAnim(slot, walking ? 138 : 137); break;
    case 111: SetAnim(slot, walking ? 94  : 93);  break;
    case 112: SetAnim(slot, walking ? 96  : 95);  break;
    case 113: SetAnim(slot, walking ? 98  : 97);  break;
    case 114: SetAnim(slot, walking ? 104 : 103); break;
    case 115: SetAnim(slot, walking ? 102 : 101); break;
    case 116: SetAnim(slot, walking ? 106 : 105); break;
    case 117: SetAnim(slot, walking ? 108 : 107); break;
    case 118: SetAnim(slot, walking ? 100 : 99);  break;
    case 119: SetAnim(slot, walking ? 110 : 109); break;
    case 120: SetAnim(slot, walking ? 112 : 111); break;
    case 121: SetAnim(slot, walking ? 114 : 113); break;
    case 122: SetAnim(slot, walking ? 116 : 115); break;
    case 123: SetAnim(slot, walking ? 118 : 117); break;
    case 124: SetAnim(slot, walking ? 120 : 119); break;
    case 125: SetAnim(slot, walking ? 122 : 121); break;
    case 126: SetAnim(slot, 123); break;
    case 127: SetAnim(slot, 124); break;
    case 128: SetAnim(slot, 128); break;
    case 129: SetAnim(slot, 125); break;
    case 130: SetAnim(slot, 126); break;
    case 131: SetAnim(slot, 127); break;
    default:
        // Desconocido — setea la acción cruda como anim_state (coincide con IDA
        // SetAction default branch).
        SetAnim(slot, action);
        break;
    }
}

// 0x07
void NetRecv_07(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PMSG_EFFECT_STATE_SEND (C1:07): [state][effect:LE16]
    // [index:BE16].  MuEmu sends this whenever a persistent
    // effect starts or ends (including Mana Shield = 0x100).
    if (Size < 8) return;
    const BYTE state = Msg[3];
    const WORD effect = (WORD)(Msg[4] | (Msg[5] << 8));
    const WORD entityKey = (WORD)((Msg[6] << 8) | Msg[7]);
    BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
    if (!entityBase) return;
    bool foundEntity = false;
    for (int slotIndex = 0; slotIndex < 400; ++slotIndex) {
        BYTE* entity = entityBase + slotIndex * 0x394;
        // FindCharacterIndex (0045AC80) requires an active slot.
        if (!entity[0] || *(WORD*)(entity + 476) != entityKey) continue;

        // ProtocolCore 004389A0 llama a Insert/Clear sólo cuando el
        // requested mask changes the entity's physical-effects
        // bitmap. Esto importa para los efectos con owner visual:
        // un arranque duplicado no tiene que recrear sus partículas/joints.
        const DWORD currentEffects = *(DWORD*)(entity + 120);
        if (state == 1) {
            if ((currentEffects & effect) != effect) {
                ApplyPersistentSkillEffect97k(entity, effect, 1);
            }
        } else if ((currentEffects & effect) == effect) {
            ApplyPersistentSkillEffect97k(entity, effect, 0);
        }
        foundEntity = true;
        break;
    }
    NetLog("NET:    EffectState state=%u effect=0x%03X key=%u entity=%s",
           (unsigned)state, (unsigned)effect, (unsigned)entityKey,
           foundEntity ? "found" : "missing");
}

// 0x19
void NetRecv_19(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PacketHandler_0x19 Skill — server tells client about skill effects.
    // Format: [C1][size][0x19][skill_idx][src_id_hi][src_id_lo][tgt_id_hi][tgt_id_lo]
    // Delega en PacketHandler_0x19 (Combat/Skills.cpp), que tiene
    // the full 30+ skill type dispatch (Poison/Ice/Lightning/Combo/etc.).
    if (Size < 8) return;
    NetLog("NET:  → 0x19 Skill idx=%d size=%d", Msg[3], Size);
    extern void PacketHandler_0x19(BYTE* pkt);
    PacketHandler_0x19((BYTE*)Msg);
}

// 0x1E
void NetRecv_1E(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveMagicContinue @ 0042CD10. El broadcast del server es
    // [C3][size][1E][skill][casterHi][casterLo][x][y][dir].
    // El casteo local ya es dueño de su animación; las entidades remotas
    // eligen acá la acción específica del skill.
    if (Size < 9) return;
    const WORD casterKey = (WORD)((Msg[4] << 8) | Msg[5]);
    BYTE* caster = nullptr;
    BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int slotIndex = 0; slotIndex < 400; ++slotIndex) {
        BYTE* candidate = entityBase + slotIndex * 0x394;
        if (*(WORD*)(candidate + 476) == casterKey) {
            caster = candidate;
            break;
        }
    }
    if (!caster) return;

    const BYTE skill = Msg[3];
    // ReceiveMagicContinue @ 0042CD10 stores the received skill
    // en c+770 antes de armar c+757. CharacterAnimation después
    // llama a AttackStage, que despacha sus efectos visuales desde
    // this exact byte (Evil Spirit included).
    caster[770] = skill;
    caster[776] = Msg[6];
    caster[777] = Msg[7];
    if (caster == (BYTE*)DAT_07abf5d8) {
        caster[757] = 1;
        *(WORD*)(caster + 784) = 0xFFFF;
        caster[756] = 0;
        return;
    }

    if (*(WORD*)(caster + 2) == 390) {
        switch (skill) {
        case 10: SetAction((int)caster, 90); break;
        case 12: SetAction((int)caster, 88); break;
        case 14: SetAction((int)caster, 89); break;
        case 24:
        case 52: SetPlayerAttack((int)caster, 0, 0, 0); break;
        case 41:
        case 55: SetAction((int)caster, 61); break;
        case 42: SetAction((int)caster, 62); break;
        case 43: SetAction((int)caster, 67); break;
        case 47: SetAction((int)caster, 66); break;
        case 56: SetAction((int)caster, 81); break;
        default: SetPlayerMagic((int)caster); break;
        }
    } else {
        SetPlayerAttack((int)caster, 0, 0, 0);
    }
    *(DWORD*)(caster + 264) = 0;
    caster[757] = 1;
    *(WORD*)(caster + 784) = 0xFFFF;
    caster[756] = 0;
}

// 0x1B
void NetRecv_1B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ProtocolCore @ 004389A0: PMSG_SKILL_CANCEL_SEND
    // [C1][6][1B][skill][casterHi][casterLo]. Clear the exact
    // flag de efecto físico asociado al skill cancelado.
    if (Size < 6) return;
    DWORD effectMask = 0;
    switch (Msg[3]) {
    case 1:    effectMask = 1; break;
    case 7:    effectMask = 2; break;
    case 0x10: effectMask = 0x100; break;
    case 0x1B: effectMask = 8; break;
    case 0x1C: effectMask = 4; break;
    case 0x30: effectMask = 0x10; break;
    case 0x33: effectMask = 0x20; break;
    case 0x37: effectMask = 0x40; break;
    default: break;
    }
    if (effectMask == 0) return;

    const WORD casterKey = (WORD)((Msg[4] << 8) | Msg[5]);
    BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int slotIndex = 0; slotIndex < 400; ++slotIndex) {
        BYTE* caster = entityBase + slotIndex * 0x394;
        if (caster[0] && *(WORD*)(caster + 476) == casterKey) {
            // La misma limpieza que ReceiveEffectState(state=0): apenas
            // clearing +120 leaves Mana Shield's joint 266 (and
            // other owner-bound visuals) alive after cancellation.
            ApplyPersistentSkillEffect97k(caster, (WORD)effectMask, 0);
            break;
        }
    }
}

// 0x16
void NetRecv_16(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveDieExp @ 0x0042DB60 — port FIEL.  Es la variante CHICA
    // del 0x9C: mismo cuerpo, pero con la experiencia en un WORD en
    // vez de los campos View* de 32 bits.
    //
    // IDA L105-207:
    //   Key    = Rb[4] + (Rb[3] << 8);
    //   Exp    = Rb[6] + (Rb[5] << 8);
    //   Damage = Rb[8] + (Rb[7] << 8);
    //   Index  = FindCharacterIndex(Key & 0x7FFF);
    //   c      = CharactersClient + 916 * Index;
    //   Color  = { 1.0, 0.6, 0.0 };
    //   if ( Key & 0xFFFF8000 ) { SetPlayerDie(c); CreatePoint(...); }
    //   else { Hero+756 = 2; Hero+758 = Damage; Hero+784 = Index;
    //          CreatePoint(...); }
    //   c+765 = 1;  c+748 = 0;
    //   CharacterAttribute+16 += Exp;
    //   if ( Exp > 0 ) { sprintf(Buffer, GlobalText[486], Exp);
    //                    UIChatLogWindow_AddText(...); }
    //
    // MuEmu NO manda este opcode (usa el 0x9C, GCMonsterDieSend →
    // PMSG_REWARD_EXPERIENCE_SEND con header.setE(0x9C)), asi que en
    // la practica no corre.  Se porta igual porque lo que habia antes
    // era una rama inventada ("teleport begin/end + kill confirm") que
    // escribia el dead_flag `target[0x2FD] = 1` sobre un indice sin
    // validar — y +765 es justo el filtro de "vivo" del barrido de
    // sub_45FEC0, o sea marcaba entidades como muertas y las volvia
    // invisibles para el reporte de blancos del 0x1D.
    if (Size < 9) { NetLog("NET:  → 0x16 DieExp size=%d (corto)", Size); return; }
    {
        const int   key   = Msg[4] + (Msg[3] << 8);
        const DWORD exp   = (DWORD)(Msg[6] + (Msg[5] << 8));
        const int   dmg   = Msg[8] + (Msg[7] << 8);
        const int   index = FindCharacterIndex(key & 0x7FFF);

        NetLog("NET:  → 0x16 DieExp key=%04X idx=%d exp=%u dmg=%d",
               key & 0x7FFF, index, exp, dmg);

        float color[3] = { 1.0f, 0.6f, 0.0f };   // naranja
        if (index >= 0 && index < 400 && DAT_07abf5d0) {
            BYTE* c = (BYTE*)(uintptr_t)DAT_07abf5d0 + 916 * index;
            if (key & 0xFFFF8000) {
                extern void __cdecl SetPlayerDie(int c_in);   // SetPlayerDie
                SetPlayerDie((int)(intptr_t)c);
            } else if (DAT_07abf5d8) {
                BYTE* hero = (BYTE*)DAT_07abf5d8;
                *(BYTE*) (hero + 756) = 2;        // gate de las esferas de EXP
                *(WORD*) (hero + 758) = (WORD)dmg;
                *(WORD*) (hero + 784) = (WORD)index;
            }
            CreatePoint((float*)(c + 16), dmg, color, 15.0f);
            *(BYTE*)(c + 765) = 1;   // dead_flag (+0x2FD)
            *(BYTE*)(c + 748) = 0;
        }

        if (CharacterAttribute)
            *(DWORD*)((BYTE*)(uintptr_t)CharacterAttribute + 16) += exp;

        if ((int)exp > 0) {
            char Buffer[100];
            sprintf(Buffer, GlobalText[486], exp);
            UIChatLogWindow_AddText(nullptr, Buffer, 1);
        }
    }
}

// 0x17
void NetRecv_17(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveDie @ 0042F030. El paquete es exactamente la clave de la
    // entidad en los bytes 3..4; no es una actualización de HP/escudo. IDA
    // marca la entidad como muerta, detiene su movimiento y despacha
    // the model-specific death animation.
    if (Size < 5) {
        NetLog("NET:  → 0x17 Die size=%d (too small, skip)", Size);
        return;
    }
    const WORD entityId = (WORD)((Msg[3] << 8) | Msg[4]);
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    BYTE* entity = nullptr;
    if (entityId == g_HeroKey && DAT_07abf5d8) {
        entity = (BYTE*)DAT_07abf5d8;
    } else if (basePtr) {
        for (int slot = 0; slot < 400; ++slot) {
            BYTE* candidate = basePtr + slot * 0x394;
            if (candidate[0] && *(WORD*)(candidate + 0x1DC) == entityId) {
                entity = candidate;
                break;
            }
        }
    }
    if (!entity) {
        NetLog("NET:  → 0x17 Die id=%d (entity missing)", entityId);
        return;
    }

    // ReceiveDie exacta (0042F030): sólo arranca el contador de muerte
    // y detiene el movimiento. MoveCharacter incrementa +765 y
    // despacha SetPlayerDie en el frame terminal nativo.
    entity[765] = 1;
    entity[748] = 0;

    // Blood Castle: caer del puente al morir (IDA 0x42F030 L18-56).
    //   c+405 = m_bActionStart (lo consume MoveCharacter L572 y
    //           RenderCharacter L361/L757)
    //   c+192/196 Gravity/Velocity . c+200 spin . c+204/216 caida
    //   c+408..416 m_vDownAngle   . c+420..428 m_vDeadPosition
    // El tile tiene que tener el bit 0x20 (TW_ACTION); la direccion
    // de la caida sale de si el tile de al lado (indice +1 o -1)
    // tiene 0x08 (TW_NOGROUND).
    entity[405] = 0;
    if ((int)gMapManager.GetCurrentMap() >= 11 && (int)gMapManager.GetCurrentMap() <= 16) {
        const int gx = (int)(*(float*)(entity + 16) * 0.01f);
        const int gy = (int)(*(float*)(entity + 20) * 0.01f);
        const int wallIndex = ((gy & 0xFF) << 8) | (gx & 0xFF);
        if ((TerrainWall[wallIndex] & 0x20) == 0x20) {
            entity[772] = 0;
            entity[405] = 1;
            *(float*)(entity + 216) = (float)(rand() % 10) + 10.0f;
            *(float*)(entity + 204) = (float)(rand() % 20) + 20.0f;
            const float angle = (float)(rand() % 10) + 85.0f;
            if ((TerrainWall[(wallIndex + 1) & 0xFFFF] & 8) == 8) {
                *(float*)(entity + 416) = -angle;
                *(int*)(entity + 408) = 0;
                *(int*)(entity + 412) = 0;
            } else if ((TerrainWall[(wallIndex - 1) & 0xFFFF] & 8) == 8) {
                *(float*)(entity + 416) = angle;
                *(int*)(entity + 408) = 0;
                *(int*)(entity + 412) = 0;
            }
            *(int*)(entity + 36)  = *(int*)(entity + 416);
            *(float*)(entity + 192) = (float)(rand() % 6) + 8.0f;
            *(float*)(entity + 196) = (float)(-(rand() % 2)) + 13.0f;
            const int spin = rand();
            *(int*)(entity + 424) = *(int*)(entity + 20);
            *(int*)(entity + 428) = *(int*)(entity + 24);
            *(int*)(entity + 420) = *(int*)(entity + 16);
            *(float*)(entity + 200) = (float)(spin % 45);
        }
        if (DAT_07abf5d8 && entity == (BYTE*)DAT_07abf5d8) {
            clearMatchInfo();   // clearMatchInfo
        }
    }

    const int entitySlot = basePtr ? (int)((entity - basePtr) / 0x394) : -1;
    NetLog("NET:  → 0x17 Die id=%d slot=%d fall=%d",
           entityId, entitySlot, entity[405]);
}

// 0x9C
void NetRecv_9C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveDieExpLarge @ 0x42E5C0 — el paquete que MuEmu manda de
    // verdad al morir un mob: GCMonsterDieSend (Protocol.cpp:1811)
    // usa PMSG_REWARD_EXPERIENCE_SEND con header.setE(0x9C).
    // (El 0x16 / PMSG_MONSTER_DIE_SEND del Protocol.h no se usa.)
    //
    // Layout con el padding del struct — el cliente lo lee EXACTO:
    //   +3,+4  BYTE  index[2]        (bit 0x8000 = el muerto era player)
    //   +5     padding
    //   +6..9  WORD  experience[2]   (HW en +6, LW en +8)
    //   +10,11 BYTE  damage[2]
    //   +12..  ViewDamageHP / ViewExperience / ViewNextExperience
    //
    // IDA L102-123:
    //   v28 = Rb[4] + (Rb[3] << 8);
    //   v83 = *(WORD*)(Rb+8) + (*(WORD*)(Rb+6) << 16);   // exp 32-bit
    //   v30 = Rb[11] + (Rb[10] << 8);                    // damage
    //   Index = FindCharacterIndex(v28 & 0x7FFF);
    //   if (v28 & 0x8000) { SetPlayerDie(c); CreatePoint(...); }
    //   else { Hero+756 = 2; Hero+758 = v30; Hero+784 = Index;
    //          CreatePoint(...); }
    //   c+765 = 1; c+748 = 0;
    //   CharacterAttribute+16 += v83;
    //   if (v83 > 0) { sprintf(GlobalText[486], v83); chat log }
    //
    // `Hero+756 = 2` es lo que dispara las DOS esferas de EXP que
    // van del mob al jugador: MoveCharacter (0x449900 L2450) hace
    //   if (c+756 == 2) { CreateJoint(1258, ..., subtype 0);
    //                     CreateJoint(1258, ..., subtype 1); }
    // y limpia +756/+758 al final del bloque. Ese consumidor ya
    // estaba portado (SecondPassword.cpp:3980) — faltaba el writer.
    if (Size < 12) { NetLog("NET:  → 0x9C DieExp size=%d (corto)", Size); return; }
    {
        const int   key   = Msg[4] + (Msg[3] << 8);
        const DWORD exp   = (DWORD)(*(WORD*)(Msg + 8))
                          + ((DWORD)(*(WORD*)(Msg + 6)) << 16);
        const int   dmg   = Msg[11] + (Msg[10] << 8);
        const int   index = FindCharacterIndex(key & 0x7FFF);

        NetLog("NET:  → 0x9C DieExp key=%04X idx=%d exp=%u dmg=%d",
               key & 0x7FFF, index, exp, dmg);

        float color[3] = { 1.0f, 0.6f, 0.0f };   // naranja
        if (index >= 0 && index < 400 && DAT_07abf5d0) {
            BYTE* c = (BYTE*)(uintptr_t)DAT_07abf5d0 + 916 * index;
            if (key & 0xFFFF8000) {
                // Murio un PLAYER (PvP): anim de muerte, sin EXP.
                extern void __cdecl SetPlayerDie(int c_in);
                SetPlayerDie((int)(intptr_t)c);
            } else if (DAT_07abf5d8) {
                BYTE* hero = (BYTE*)DAT_07abf5d8;
                *(BYTE*) (hero + 756) = 2;        // gate de las esferas
                *(WORD*) (hero + 758) = (WORD)dmg;
                *(WORD*) (hero + 784) = (WORD)index;
            }
            CreatePoint((float*)(c + 16), dmg, color, 15.0f);
            *(BYTE*)(c + 765) = 1;   // dead_flag (+0x2FD)
            *(BYTE*)(c + 748) = 0;
        }

        if (CharacterAttribute)
            *(DWORD*)((BYTE*)(uintptr_t)CharacterAttribute + 16) += exp;

        if ((int)exp > 0) {
            char Buffer[100];
            sprintf(Buffer, GlobalText[486], exp);
            UIChatLogWindow_AddText(nullptr, Buffer, 1);
        }
    }
}

// 0x1A
void NetRecv_1A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveMagicPosition @ 0x0042D780 (port FIEL).
    // Server broadcasts a magic-area-attack:
    //   Msg[3..4] = caster entity ID (BE)
    //   Msg[5..6] = ID del skill mágico (BE) (se usa para el efecto visual)
    //   Msg[7]    = direction byte (unused in viz)
    //   Msg[8]    = target count
    //   Msg[9+i*2..10+i*2] = target IDs (BE)
    // Por cada objetivo: anim de shock + popup de daño.
    if (Size < 9) return;
    WORD casterId = ((WORD)Msg[3] << 8) | Msg[4];
    BYTE count = Msg[8];
    NetLog("NET:  → 0x1A MagicPosition caster=%d targets=%d", casterId, count);

    // Find caster, set magic-cast animation
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    BYTE* caster = nullptr;
    for (int s = 0; s < 400; ++s) {
        BYTE* sp = basePtr + s * 0x394;
        if (sp[0] && *(WORD*)(sp + 0x1dc) == casterId) {
            caster = sp; break;
        }
    }
    if (caster) {
        CreateMagicShiny97k(caster, 0);
        caster[0x106] = caster[0x105];
        caster[0x105] = 90;            // cast anim (action 90)
        *(float*)(caster + 0x108) = 0.0f;
        caster[770] = Msg[5];           // ReceiveMagicPosition
        caster[0x2F5] = 1;             // attack pending
        PlayBuffer(88, 0, 0);          // magic cast sound
    }

    // Iterate targets — apply shock + damage popup
    if (Size >= 9 + (int)count * 2) {
        for (int i = 0; i < count; ++i) {
            WORD tgtId = ((WORD)Msg[9 + i*2] << 8) | Msg[10 + i*2];
            BYTE* tgt = nullptr;
            if (tgtId == g_HeroKey && DAT_07abf5d8) {
                tgt = (BYTE*)DAT_07abf5d8;
            } else {
                for (int s = 0; s < 400; ++s) {
                    BYTE* sp = basePtr + s * 0x394;
                    if (sp[0] && *(WORD*)(sp + 0x1dc) == tgtId) {
                        tgt = sp; break;
                    }
                }
            }
            if (!tgt) continue;
            // Daño cacheado en +760 (0x2F8) por el 0x15 — se usa como visual
            WORD dmg = *(WORD*)(tgt + 760);
            // ReceiveMagicPosition tira el mismo 50/50 de reacción
            // que usaba el original antes de dibujar el daño en área.
            const unsigned r = (unsigned)rand() & 0x80000001u;
            const bool shock = (r == 0) ||
                               ((int)r < 0 &&
                                ((((char)r - 1) | (int)0xFFFFFFFE) == -1));
            if (shock) SetPlayerShock((int)(uintptr_t)tgt, dmg);
            if (dmg) {
                float pos[3];
                pos[0] = *(float*)(tgt + 0x10);
                pos[1] = *(float*)(tgt + 0x14);
                pos[2] = *(float*)(tgt + 0x18);
                float color[3] = { 1.0f, 0.6f, 0.0f };
                CreatePoint(pos, (int)dmg, color, 15.0f);
            }
            // Hero damaged: decrement HP
            if (tgtId == g_HeroKey && DAT_07cf1ff4) {
                WORD* pHP = (WORD*)((BYTE*)(uintptr_t)DAT_07cf1ff4 + 28);
                if (dmg && dmg <= *pHP) *pHP -= dmg;
                else if (dmg) *pHP = 0;
            }
        }
    }
}
