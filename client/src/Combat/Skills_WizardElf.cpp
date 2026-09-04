// Skills_WizardElf.cpp
//
// Extracted from stubs_game.cpp.  Owns wizard/elf skill dispatch helpers.
// Public entry points retain IDA provenance in their leading comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" BYTE OffsetInventoryItems[];
extern "C" BYTE Inventory[];
static BYTE ResolveQueuedSkillId97k()
{
    // UseSkillWizard @ 488A53 always dereferences the selected slot in
    // CharacterAttribute; its result is not conditional on the queued-action
    // flag.  The flag only controls how Attack/Action seed the slot.
    BYTE* charAttr = (BYTE*)DAT_07cf1ff4;
    if (charAttr) {
        return charAttr[DAT_07d7809c + 87];
    }
    return (BYTE)DAT_07d7809c;
}

static void SendSkillPacket19_Local(BYTE skillId, WORD targetKey);
static void SendSkillPacket1E_Local(BYTE skillId, BYTE gridX, BYTE gridY, BYTE dir, BYTE dist, BYTE angle, WORD targetKey);

static void SendMuEmuEncryptedPacket(BYTE* pkt, int len)
{
    if (!pkt || len <= 0) return;
    Net_SendSmallPacket(pkt, len);
}

// UseSkillWizard @ 0x004889D0 (~1227 lines)
// Wizard skill execution. Reads skill ID from CharacterAttribute skill table,
// builds XOR-encrypted C1 packets for different wizard skill types, sends via socket.
// Special handling for skills 43/47, ground-target skill 13, and the direct
// target set recovered from the switch at 0x489006: 1,2,3,4,7,11,17.
//
// ~70% of original 1227 lines is anti-tamper hash table operations
// (FUN_00403f80, FUN_004041e0, FUN_00404280, FUN_00404330, FUN_00404370,
//  FUN_00404400) and XOR key init + dead forward/reverse loops — all skipped.
//
// IDA: UseSkillWizard @ 0x004889D0. Ghidra shows 63 phantom stack params
// (unaff_retaddr etc.) — anti-tamper obfuscation.
// Real params: c = CHARACTER* (hero entity ptr), o = OBJECT* (hero object ptr).
// Correspondencia: skillId = el byte de CharacterAttribute[dword_7D7809C + 87];
//   targetIdx = MovementSkillTarget (0x07D780A0); angle = retorno de CreateAngle
//   (0x0043E050, que el decompile llama `Movement_Tick`); targetKey = entidad+476.
void __cdecl Combat_UseWizardSkill(DWORD c, DWORD o) {
    // IDA: sub_4889D0 @ 0x004889D0.  `c` is the queued caster CHARACTER
    // (IDA: c); `o` is its OBJECT (IDA: a2).  The skill selector itself is
    // deliberately global in the original, not a member of either parameter.
    // 004889D0 resolves the selected skill from the CharacterAttribute slot
    // when dword_7D78098 is active.  Do not replace it with the local runtime
    // cache: that cache survives target/mouse transitions and can dispatch a
    // previously selected spell after the user has changed slot.
    UINT skillId = (UINT)ResolveQueuedSkillId97k();

    // 004889E6: these warrior-only IDs exit the wizard helper before the
    // movement packet, target snapshot, or animation.  Attack routes them
    // through Item_Equip/UseSkillWarrior instead.
    if (skillId == 0x2b || skillId == 0x2f) {
        return;
    }

    // 0x488A90: compact facing packet, present for every non-15 spell.  It
    // shares the native C1 sender used by the warrior helper; it is not the
    // path-list sender at 0x491C40.
    if (skillId != 15) {
        BYTE movement[6] = { 0xC1, 6, 0x10,
            *(BYTE*)((BYTE*)(uintptr_t)Hero + 904), *(BYTE*)((BYTE*)(uintptr_t)Hero + 908),
            (BYTE)(16 * (((int)((*(float*)((BYTE*)(uintptr_t)Hero + 36) + 22.5f) * 0.022222223f + 1.0f)) & 7)) };
        Net_SendC1Packet(movement, sizeof(movement));
        *(BYTE*)((BYTE*)(uintptr_t)c + 748) = 0;
    }

    // ── Set target position from CharactersClient[MovementSkillTarget] ──
    char* heroEntity = (char*)(DWORD)c;
    int targetIdx = (int)DAT_07d780a0;  // MovementSkillTarget
    DWORD entityBase = DAT_07abf5d0;    // CharactersClient array base

    // Target entity position: entityBase + targetIdx * 0x394 + offset
    // Ghidra: CharactersClient[_MovementSkillTarget].Object.Position[0/1/2]
    float targetPosX = *(float*)(entityBase + targetIdx * 0x394 + 0x10);
    float targetPosY = *(float*)(entityBase + targetIdx * 0x394 + 0x14);
    float targetPosZ = *(float*)(entityBase + targetIdx * 0x394 + 0x18);

    // Store target position at entity offset +0x314 (TargetPosition[3])
    *(float*)(heroEntity + 0x314) = targetPosX;
    *(float*)(heroEntity + 0x318) = targetPosY;
    *(float*)(heroEntity + 0x31C) = targetPosZ;

    // Compute facing angle toward target
    // CreateAngle(heroX, heroY, targetX, targetY)
    float heroX = *(float*)((char*)(DWORD)o + 0x10);
    float heroY = *(float*)((char*)(DWORD)o + 0x14);
    typedef float (__cdecl *CreateAngleFn)(float, float, float, float);
    float angle = ((CreateAngleFn)&FUN_0043e050)(heroX, heroY, targetPosX, targetPosY);
    *(float*)((char*)(DWORD)o + 36) = angle;

    // ── Switch on skill ID ──
    switch (skillId) {
    case 1: case 2: case 3: case 4:
    case 7: case 0xb: case 0x11:
    {
        // GM name check: compare Hero->ID with "webzen" (DAT_00559d94)
        // Hero->ID at entity offset +0x1C1
        // In original: strlen-based strstr scan. If hero name contains "webzen", skip.
        // This is an anti-impersonation check — GM accounts get special treatment.
        char* heroName = Hero ? (char*)(Hero + 449) : (char*)((DWORD)c + 449);
        {
            // Simple strstr check for "webzen" in hero name
            const char* gmStr = DAT_00559d94;
            int gmLen = (int)strlen(gmStr);
            int nameLen = (int)strlen(heroName);
            int diff = nameLen - gmLen;
            bool isGM = false;
            for (int i = 0; i <= diff && !isGM; i++) {
                bool match = true;
                for (int j = 0; j < gmLen; j++) {
                    if (heroName[i + j] != gmStr[j]) { match = false; break; }
                }
                if (match) isGM = true;
            }
            if (isGM) break;  // GM name found — skip packet send
        }

        // 300ms cooldown check on magic tick
        DWORD now = GetTickCount();
        DWORD elapsed = now - DAT_05826cf4;
        // Ghidra: abs(elapsed) > 300
        if (elapsed > 0x80000000) elapsed = (DWORD)(-(int)elapsed);  // abs
        if ((int)elapsed <= 300) break;

        DAT_05826cf4 = GetTickCount();  // g_dwLatestMagicTick = now

        // Get target entity key (CharactersClient[MovementSkillTarget].Key)
        // Key is at entity offset +0x??? — Ghidra shows CharactersClient[target].Key
        // In the entity struct, Key is typically near the beginning. Ghidra accesses
        // it as a DWORD. From the struct: stride 0x394, key at some offset.
        // Ghidra: (byte)(CharactersClient[_MovementSkillTarget].Key >> 8) and
        //         (byte)(CharactersClient[_MovementSkillTarget].Key)
        // The Key field is at entity+0x00 or a named offset. Based on CreateCharacter
        // which takes Key as param and stores it, it's at entity+0x???
        // From Monster.cpp: FindCharacterIndex searches by Key — likely at offset 0x00
        // Actually, looking at Ghidra: CharactersClient[idx].Key — this is a named field.
        // Entity struct in structs.h doesn't have explicit Key offset, but from entity
        // struct the first fields are type(+0x02) then positions(+0x10).
        // Key is probably stored at another offset. Let's check: in Game_EnterWorldTick,
        // CreateCharacter stores Key as first param. Ghidra accesses it as a member.
        // For now, assume Key is stored at a known offset in the entity struct.
        // From CreateCharacter signature: (int Key, int Type, BYTE PosX, BYTE PosY, float Rot)
        // The entity index IS the key in some sense, but Ghidra shows .Key as a field.
        // Looking at Combat.cpp and other code: entity key is used as network ID.
        // From DAT_07abf5d0 + idx * 0x394 — the Key field in Ghidra is likely at offset 0x00.
        WORD targetKey = *(WORD*)(entityBase + targetIdx * 0x394 + 476); // IDA: *v38

        // Real server recv:
        //   PMSG_SKILL_ATTACK_RECV { PBMSG_HEAD // C3:19, BYTE skill, BYTE index[2] }
        SendSkillPacket19_Local((BYTE)skillId, targetKey);
        break;
    }

    case 0xd:
    {
        // GM name check: compare Hero->ID with "webzen" (DAT_00559d9c)
        char* heroName = Hero ? (char*)(Hero + 449) : (char*)((DWORD)c + 449);
        {
            const char* gmStr = DAT_00559d9c;
            int gmLen = (int)strlen(gmStr);
            int nameLen = (int)strlen(heroName);
            int diff = nameLen - gmLen;
            bool isGM = false;
            for (int i = 0; i <= diff && !isGM; i++) {
                bool match = true;
                for (int j = 0; j < gmLen; j++) {
                    if (heroName[i + j] != gmStr[j]) { match = false; break; }
                }
                if (match) isGM = true;
            }
            if (isGM) break;  // GM name found — skip
        }

        // Set CurrentSkill
        DAT_05826d10 = skillId;  // CurrentSkill

        // The sole duration case in this helper is skill 13.  It uses the
        // cached target world point established immediately above.
        BYTE gridX = (BYTE)(int)(*(float*)(heroEntity + 788) * 0.01f);
        BYTE gridY = (BYTE)(int)(*(float*)(heroEntity + 792) * 0.01f);

        // PMSG_DURATION_SKILL_ATTACK uses a 0..255 facing byte. The 97k
        // Blast sender leaves distance and angle at zero.
        // 0x4889D0 writes three literal zero bytes after x/y; it does not
        // append the target key nor the current facing angle in this client.
        // DLL SendContinueBlast (Patchs.cpp:1385): (dir, 0, 0, index) con
        // index = -1 cuando CheckAttack() falla.  0 es un slot de objeto VALIDO,
        // asi que mandar 0 haria que el server aplique el skill sobre gObj[0].
        SendSkillPacket1E_Local((BYTE)skillId, gridX, gridY, 0, 0, 0, 0xFFFF);
        break;
    }

    default:
        // Skills 5,6,8-10,12,14-16 and every other non-listed id leave through
        // the default arm of the original jump table without a packet.
        // Ghidra: goto switchD_00489006_caseD_5 (exit without packet)
        return;
    }

    // SetPlayerMagic(c) — sets cast animation on hero entity
    // Ghidra: SetPlayerMagic @ 0x00444a80
    // If entity type != 0x186: alternate between action 3 and 4 based on combo counter % 3
    // If entity type == 0x186: SetAttackSpeed, then class-specific action
    FUN_00444a80((int)c);
}

// ── SkillElf support ──────────────────────────────────────────────────────────

static void SendSkillPacket19_Local(BYTE skillId, WORD targetKey)
{
    BYTE pktBuf[6];
    pktBuf[0] = 0xC1;
    pktBuf[1] = 6;
    pktBuf[2] = 0x19;
    pktBuf[3] = skillId;
    pktBuf[4] = (BYTE)(targetKey >> 8);
    pktBuf[5] = (BYTE)(targetKey & 0xFF);
    SendMuEmuEncryptedPacket(pktBuf, 6);
}

// GetDestValue - DLL Source/Client/Main/Util.cpp:323 (identico al helper del
// cliente 5.2 en source/wsclientinline.h:615): nibble alto = delta X, nibble
// bajo = delta Y, ambos clampeados a [-8, 7].
//
// 2026-09-02: NO se unifico con Combat_GetDestValue97k (Combat.cpp), que si se
// paso a la forma del binario.  Motivo: de este sitio no hay expresion que
// comparar -- el decompile de SkillElf (0x0048BD70 LABEL_68) llega plegado
// (`if...`) y no muestra los appends del payload.  Lo unico seguro es que el
// hook que reemplaza este sitio, CPatchs::SendContinueTripleShot
// (Patchs.cpp:1394-1444), pasa `dest = GetDestValue(x, y, TargetX, TargetY)`.
// Da igual funcionalmente: MuEmu no lee `dis` (SkillManager.cpp:2047 solo
// propaga x, y, dir, angle e index[]).  Se deja como esta hasta poder leer los
// appends en el disassembly.
static BYTE Combat_GetDestValue97kExt(int xPos, int yPos, int xDst, int yDst)
{
    int dx = xDst - xPos;
    int dy = yDst - yPos;
    if (dx < -8) dx = -8;
    if (dx >  7) dx =  7;
    if (dy < -8) dy = -8;
    if (dy >  7) dy =  7;
    return (BYTE)((((BYTE)(dx + 8)) << 4) | (((BYTE)(dy + 8)) & 0x0F));
}

static void SendSkillPacket1E_Local(BYTE skillId, BYTE gridX, BYTE gridY, BYTE dir, BYTE dist, BYTE angle, WORD targetKey)
{
    // DESVIACION DE PROTOCOLO (servidor MuEmu) — ver la nota extensa en
    // Combat_SendDuration1E_97k (src/Combat/Combat.cpp).  El 0.97k vanilla manda
    // 9 bytes; PMSG_DURATION_SKILL_ATTACK_RECV son 11 y el server lee `index[]`
    // siempre, asi que sin esos dos bytes le pega a una entidad al azar.
    BYTE pktBuf[11];
    pktBuf[0] = 0xC1;
    pktBuf[1] = 11;
    pktBuf[2] = 0x1E;
    pktBuf[3] = skillId;
    pktBuf[4] = gridX;
    pktBuf[5] = gridY;
    pktBuf[6] = dir;                                   // DLL: dir
    pktBuf[7] = dist;                                  // DLL: dis
    pktBuf[8] = angle;                                 // DLL: angle
    pktBuf[9]  = (BYTE)((targetKey >> 8) & 0xFF);      // DLL: HIBYTE(target)
    pktBuf[10] = (BYTE)(targetKey & 0xFF);             // DLL: LOBYTE(target)
    SendMuEmuEncryptedPacket(pktBuf, sizeof(pktBuf));
}

// GetSkillInformation @ 0x0047E7A0 — reads skill table entry for given type/level.
// Outputs mana cost, distance, and AG (SkillMana) cost via out-pointers.
// 0047E7A0 uses the clear SkillAttribute table, with 0x28-byte entries.
void __cdecl GetSkillInformation(int iType, int iLevel, char* lpszName, int* piMana, int* piDistance, int* piSkillMana) {
    (void)iLevel;
    if (iType < 0 || iType >= 64) {
        if (lpszName) lpszName[0] = '\0';
        if (piMana) *piMana = 0;
        if (piDistance) *piDistance = 0;
        if (piSkillMana) *piSkillMana = 0;
        return;
    }
    // DAT_07cf1ff8 is the encrypted/shadow counterpart.  The loader mutates
    // its +0x26 field, so it must not drive combat costs/ranges.
    BYTE* const entry = (BYTE*)SkillAttribute.Raw + iType * 0x28;

    if (lpszName != NULL) {
        strncpy_s(lpszName, 256, (const char*)entry, 31);
        lpszName[31] = '\0';
    }
    if (piMana != NULL) {
        *piMana = (int)*(WORD*)(entry + 0x22);
    }
    if (piDistance != NULL) {
        // 0047E7A0: SkillAttribute[40*iType + 38], not byte 39.
        *piDistance = (int)*(BYTE*)(entry + 0x26);
    }
    if (piSkillMana != NULL) {
        // 0047E7A0 reads the unsigned WORD at +36 (the 18th WORD).
        *piSkillMana = (int)*(WORD*)(entry + 0x24);
    }
}


// IDA: SkillElf @ 0x0048BD70 (6351 bytes) — bool __cdecl SkillElf(DWORD c, DWORD pItem)
// Elf class skill execution. Handles heal, buff, arrow skills.
// Validates target, builds skill packet, handles multi-arrow, spawns VFX.
//
// NOTE: Ghidra shows 63 phantom stack params (in_stack_00000020..in_stack_000000ff)
//   — these are anti-tamper obfuscation, not real parameters.
//   in_stack_00001b48 = c (CHARACTER* pointer, 1st real param)
//   in_stack_00001b4c = pItem (CHARACTER_ATTRIBUTE* pointer, 2nd real param)
//
// The function iterates over skills in the pItem (CharacterAttribute) skill list,
// checking which one matches the currently selected skill on the hero.
// For each matching skill:
//   - If mana is insufficient: tries to use a potion (item slot 3 = arrows/potions)
//   - If AG (SkillMana) is insufficient: returns false
//   - If skill type is 0x18 (arrow/ranged): checks arrow ammo via CheckArrow,
//     validates range, computes facing angle, builds C1 skill packet with
//     XOR encryption, sends it, then calls SetPlayerAttack + CreateArrows
//
// CORRECCION DE TRAZABILIDAD (2026-09-01): este bloque decia
// "IDA: FUN_0048A180 @ 0x0048A180 — SkillElf", y 0x0048A180 es **UseSkillElf**
// (portada como Combat_UseElfSkill en Combat.cpp).  La funcion que se reconstruye
// aca es **SkillElf @ 0x0048BD70**, que es la que Attack llama como
// `SkillElf(c, i + CharacterMachine + 536)` en L1464.  functions.h ya la declaraba
// con la direccion correcta.
// Correspondencia con el decompile de 0x0048BD70:
//   charAttr / pItem   = pItem        (ITEM* equipado)
//   skillCount         = *(BYTE *)(pItem + 36)     = ITEM::SpecialNum
//   skillId            = *(BYTE *)(pItem + i + 37) = ITEM::Special[i]
//   i                  = i           (L119, la variable de bucle del binario)
//   result             = v110        (el valor de retorno)
//   gridX / gridY      = v44 / v113  (*(_DWORD *)(c + 904) / (c + 908))
//   angle              = v37         (Movement_Tick = CreateAngle @0x0043E050)
//   dir                = v52         (angulo * 0.71111113)
//   skillDistance      = v112        (SkillAttribute[40*skill + 38])
// ~60% of the original decompile is anti-tamper hash table operations
// (FUN_00403f80, FUN_004041e0, FUN_00404280, FUN_00404330, FUN_00404370,
//  FUN_00404400) and XOR key init + dead forward/reverse loops — all skipped.
bool __stdcall Combat_UseElfSkillItem(DWORD c, DWORD pItem) {
    // c = CHARACTER* (hero entity), pItem = CHARACTER_ATTRIBUTE* (char attributes)
    // Cast to usable pointers
    char* hero    = (char*)(DWORD)c;      // CHARACTER* — entity struct
    char* charAttr = (char*)(DWORD)pItem;  // CHARACTER_ATTRIBUTE* — skill/stat struct

    if (hero == nullptr || charAttr == nullptr) return false;

    // anti-tamper hash table — skipped (encrypt CharacterMachine before access)

    bool result = false;

    // CharacterAttribute = DAT_07cf1ff4, Hero = DAT_07abf5d8
    char* charAttrBase = (char*)DAT_07cf1ff4;
    char* heroEntity   = DAT_07abf5d8;

    if (charAttrBase == nullptr || heroEntity == nullptr) {
        return false;
    }

    // Attack calls SkillElf(c, CharacterAttribute + 0x218).  This is the
    // equipped item record: its skill-list metadata is at +0x24/+0x25.
    // The previous port incorrectly interpreted CharacterAttribute+0x56 as
    // this list and consequently dispatched arbitrary character bytes.
    BYTE skillCount = *(BYTE*)(charAttr + 0x24);
    if (skillCount == 0) {
        return false;
    }
    if ((int)DAT_07d780a0 < 0 || (int)DAT_07d780a0 >= 400) {
        return false;
    }

    // The same offsets are used by Attack at 49D278: current mana +0x1e
    // and current AG +0x24.
    WORD currentMana = *(WORD*)(charAttrBase + 0x1E);
    WORD currentAG = *(WORD*)(charAttrBase + 0x24);
    const BYTE selectedSkill = charAttrBase[(BYTE)*(BYTE*)(Hero + 913) + 87];

    for (int i = 0; i < (int)skillCount && i < 20; i++) {
        BYTE skillId = *(BYTE*)(charAttr + 0x25 + i);

        // 0048BE70 compares every equipped entry with the active slot before
        // inspecting costs or emitting anything.  Iterating the entire list
        // used to cast every skill carried by the item in one attack tick.
        if (skillId != selectedSkill)
            continue;

        // This helper is the native Triple-Shot path only.  The other elf
        // entries leave through LABEL_155; Heal/Greater buffs are dispatched
        // by Attack's separate paths.
        if (skillId != 24)
            continue;

        // Get skill information: mana cost and AG cost
        int manaCost = 0;
        int agCost = 0;
        GetSkillInformation((int)skillId, 1, NULL, &manaCost, NULL, &agCost);

        // Check mana: if current mana < mana cost
        if ((int)(WORD)currentMana < manaCost) {
            // Not enough mana — try to use a potion from inventory slot 3 (potions/arrows)
            // FUN_00482be0 = GetItemSlot(int category) → int slot index, or -1 if none.
            // Declaration in functions.h fixed to: int __cdecl FUN_00482be0(int).
            int slot = FUN_00482be0(3);
            if (slot == -1) continue;  // no potion found

            // Check warehouse and trade are closed
            if (DAT_07eaa119 != '\0') {  // WarehouseOpened
                UIChatLogWindow_AddText((const char*)&DAT_07e11dfc, (const char*)&DAT_07d4c89c, 2);
                continue;
            }

            // anti-tamper hash table — skipped (FUN_0043d8a0 + TradeOpened check)
            if (DAT_07eaa11b != '\0') {  // TradeOpened
                UIChatLogWindow_AddText((const char*)&DAT_07e11dfc, (const char*)&DAT_07d4c89c, 2);
                continue;
            }

            if ((int)EnableUse >= 1) continue;  // item use cooldown active
            EnableUse = 10;  // set cooldown

            // 2026-07-19: era 6 bytes. PMSG_ITEM_USE_RECV (ItemManager.h) mide
            // 5: header(3) + SourceSlot + TargetSlot. El byte extra dejaba el
            // paquete fuera de spec.
            BYTE usePkt[6];
            usePkt[0] = 0xC1;
            usePkt[1] = 5;
            usePkt[2] = 0x26;
            usePkt[3] = (BYTE)(slot + 12);
            usePkt[4] = 0;
            Net_SendSmallPacket(usePkt, 5);

            // Play sound based on item type
            // 2026-08-21: leía DAT_07ea8410, que es un DWORD suelto de 4 bytes —
            // no el pool del inventario (mismo error que ya estaba documentado
            // para FUN_004d23b0).  El grid vive en OffsetInventoryItems, stride 0x44.
            short itemType = *(short*)(OffsetInventoryItems + (size_t)slot * 0x44);
            int soundId;
            if (itemType == 0x1c0) {
                soundId = 0x21;  // healing potion sound
            } else if (itemType >= 0x1c1 && itemType <= 0x1c9) {
                soundId = 0x20;  // mana potion sound
            } else {
                continue;  // unknown item type
            }
            PlayBuffer(soundId, 0, 0);
            continue;
        }

        // Check AG (SkillMana): if current AG < AG cost
        if ((int)(WORD)currentAG < agCost) {
            // anti-tamper hash table — skipped (decrypt CharacterMachine)
            return false;
        }

        WORD targetKey = *(WORD*)((char*)(uintptr_t)DAT_07abf5d0 + (int)DAT_07d780a0 * 0x394 + 476);
        if (targetKey == 0xFFFF) {
            continue;
        }
        // 0x48BD70 reads the two encrypted grid values at c+904/c+908.
        BYTE gridX = (BYTE)*(DWORD*)(hero + 904);
        BYTE gridY = (BYTE)*(DWORD*)(hero + 908);

        // Skill type 0x18 = Triple Shot ranged-arrow path.
        {
            // Check if arrows are equipped
            char hasArrow = Combat_CheckArrowRequirement();  // CheckArrow
            if (hasArrow == '\0') continue;  // no arrows equipped

            // Attack and GetSkillInformation both read the clear 40-byte
            // SkillAttribute record.  +38 is the range field; +39 belongs to
            // the following metadata byte and was the source of an off-by-one
            // range error in the elf arrow path.
            BYTE skillDistance = SkillAttribute.Raw[(size_t)skillId * 0x28 + 0x26];

            // Get hero position
            float heroPosX = *(float*)(heroEntity + 0x10);  // Object.Position[0]
            float heroPosY = *(float*)(heroEntity + 0x14);  // Object.Position[1]

            // DESVIACION DELIBERADA respecto de IDA L197-199, que compara contra
            // los globales `TargetX`/`TargetY` (los que deja CheckTarget).  Aca se
            // usa la posicion de mundo de la entidad objetivo para que el chequeo
            // de rango y el destinatario del paquete usen el MISMO slot.
            float targetWorldX = *(float*)((char*)(uintptr_t)DAT_07abf5d0 + (int)DAT_07d780a0 * 0x394 + 0x10);
            float targetWorldY = *(float*)((char*)(uintptr_t)DAT_07abf5d0 + (int)DAT_07d780a0 * 0x394 + 0x14);

            // Check range: distance from hero to target must be within skill range
            float dx = heroPosX - targetWorldX;
            float dy = heroPosY - targetWorldY;
            float dist = SQRT(dx * dx + dy * dy);
            float maxRange = (float)(int)skillDistance * _DAT_005524f0;

            if (dist > maxRange) continue;  // target out of range

            // Compute facing angle toward target
            // CHARACTER->TargetPosition at offset +0x314 (float[3])
            float targetPosX = *(float*)(heroEntity + 0x314);
            float targetPosY = *(float*)(heroEntity + 0x318);
            float angle = ((float (__cdecl*)(float,float,float,float))FUN_0043e050)(
                heroPosX, heroPosY, targetPosX, targetPosY);  // CreateAngle

            // Set hero facing angle: Object.Angle[2] at entity offset +0x24
            *(float*)(heroEntity + 0x24) = angle;

            // GM name check: compare Hero->ID with "webzen" string
            // Hero->ID at entity offset +0x1C1
            // DAT_00559db4 = comparison string
            // (This is an anti-impersonation check — if hero name contains GM string, special path)
            // Skipping detailed string comparison; the game logic proceeds regardless

            // Set CurrentSkill global to this skill ID
            DAT_05826d10 = (DWORD)skillId;  // CurrentSkill

            // 0x48BD70 emite el C3:1E directo; el campo de direccion es
            // `Object.Direction * 0.71111113`.
            BYTE dir = (BYTE)(int)(angle * (256.0f / 360.0f));
            // DLL SendContinueTripleShot (Patchs.cpp:1394-1444) — es el UNICO
            // hook que manda los tres campos:
            //     dir   = facing / 360 * 256
            //     dest  = GetDestValue(x, y, TargetX, TargetY)
            //     angle = (facing + 180) / 360 * 256
            // El server usa `angle` (no `dir`) para el frustum de Triple Shot:
            //     SkillTripleShot(aIndex, bIndex, lpSkill, angle)
            //       -> GetSkillFrustrum(..., angle, X, Y, Radio, Range)
            // (GameServer/SkillManager.cpp:1185-1198).  Con angle = 0 el cono
            // apuntaba siempre al mismo rumbo: de ahi que Triple Shot pegara
            // "a veces".
            const BYTE dest = Combat_GetDestValue97kExt(
                                  (int)gridX, (int)gridY,
                                  (int)DAT_07e016c0, (int)DAT_07e016c4);
            const BYTE angleByte = (BYTE)(int)((angle + 180.0f) * (256.0f / 360.0f));
            SendSkillPacket1E_Local(skillId, gridX, gridY, dir, dest, angleByte, targetKey);

            // Set player attack animation
            // SetPlayerAttack(hero) — Ghidra shows 1-arg; use 4-arg decl with dummies
            FUN_00444410((int)(DWORD)heroEntity, 0, 0, 0);

            // If object type is not 0x186 (special entity), create arrow projectiles
            WORD objType = *(WORD*)(heroEntity + 0x02);  // Object.Type at offset +0x02
            if (objType != 0x186) {
                // DESVIACION respecto de IDA L223-226, que recorre
                // `CharacterAttribute + 87` hasta encontrar el skill y pasa ESE
                // indice (v87/v91) a CreateArrows.  Aca se usa el indice del bucle
                // sobre la lista del item.  Solo afecta a entidades que no son el
                // jugador (la rama esta bajo `objType != 0x186`).
                int skillIndex = i;                 // IDA: v87
                // El 6o argumento (SKKey) SI esta demostrado aca: el call site de
                // SkillElf (0x0048D488) lo empuja como literal
                //   6A 00  push 0   ; SKKey
                //   6A 01  push 1   ; Skill = 1 -> abanico de 3 flechas
                // (bytes verificados con ida_get_bytes en 0x0048D470).  A diferencia
                // del case 52 de Attack, que manda el byte de skill encolado c+770.

                // Spawn arrow visual effect
                CreateArrows_stub((DWORD)heroEntity, (DWORD)(heroEntity),
                                  (DWORD)0, (WORD)skillIndex, 1, 0);
            }
            result = true;
            continue;
        }
    }

    // anti-tamper hash table — skipped (decrypt CharacterMachine after access)
    return result;
}


// FUN_0043d670 @ 0x0043D670 (~188 lines) — hash table: read 4-byte encrypted value
// FUN_0043d670 removed — __thiscall hash table read, not needed as free function
