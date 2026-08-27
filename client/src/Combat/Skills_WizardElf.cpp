// Skills_WizardElf.cpp
//
// Extracted from stubs_game.cpp.  Owns wizard/elf skill dispatch helpers.
// Public entry points retain IDA provenance in their leading comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern "C" BYTE Inventory[];
static BYTE ResolveQueuedSkillId97k()
{
    if (DAT_07d78098 != '\0') {
        BYTE* charAttr = (BYTE*)DAT_07cf1ff4;
        if (charAttr) {
            return charAttr[DAT_07d7809c + 87];
        }
    }
    return (BYTE)DAT_07d7809c;
}

static BYTE* GetSkillRecordShadow_Local(int iType);
static BYTE* GetSkillRecordBase_Local(int iType);
static void SendSkillPacket19_Local(BYTE skillId, WORD targetKey);
static void SendSkillPacket1E_Local(BYTE skillId, BYTE gridX, BYTE gridY, BYTE dir, BYTE dist, BYTE angle, WORD targetKey);

static BYTE PackDurationDestination_Local(int x, int y, int targetX, int targetY)
{
    int deltaX = targetX - x;
    int deltaY = targetY - y;
    if (deltaX < -8) deltaX = -8;
    if (deltaX > 7) deltaX = 7;
    if (deltaY < -8) deltaY = -8;
    if (deltaY > 7) deltaY = 7;
    return (BYTE)(((deltaX + 8) << 4) | ((deltaY + 8) & 0x0F));
}

static void SendMuEmuEncryptedPacket(BYTE* pkt, int len)
{
    if (!pkt || len <= 0) return;
    Net_SendSmallPacket(pkt, len);
}

// UseSkillWizard @ 0x004889D0 (~1227 lines)
// Wizard skill execution. Reads skill ID from CharacterAttribute skill table,
// builds XOR-encrypted C1 packets for different wizard skill types, sends via socket.
// Special handling for Energy Ball (0x2b), Hellfire (0x2f), ground-target (0x0f),
// direct-target skills (1-4,7,0xb,0x11), and Decay/area skill (0xd).
//
// ~70% of original 1227 lines is anti-tamper hash table operations
// (FUN_00403f80, FUN_004041e0, FUN_00404280, FUN_00404330, FUN_00404370,
//  FUN_00404400) and XOR key init + dead forward/reverse loops — all skipped.
//
// IDA: FUN_004889D0 @ 0x004889D0 — UseSkillWizard. Ghidra shows 63 phantom stack params (unaff_retaddr etc.) — anti-tamper obfuscation.
// Real params: c = CHARACTER* (hero entity ptr), o = OBJECT* (hero object ptr).
void __cdecl UseSkillWizard_stub(DWORD c, DWORD o) {
    // --- XOR encryption key (32 bytes, hardcoded — same as login packet key) ---
    static const BYTE xorKey[32] = {
        0xe7,0x6d,0x3a,0x89, 0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6, 0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d, 0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83, 0x29,0x51,0xe8,0x56
    };

    char* charAttrBase = (char*)DAT_07cf1ff4;
    // 004889D0 resolves the selected skill from the CharacterAttribute slot
    // when dword_7D78098 is active.  Do not replace it with the local runtime
    // cache: that cache survives target/mouse transitions and can dispatch a
    // previously selected spell after the user has changed slot.
    UINT skillId = (UINT)ResolveQueuedSkillId97k();
    {
        char trace[160];
        wsprintfA(trace, "WIZ ENTER skill=%u runtime=%u target=%d active=%u",
                  (unsigned)skillId, (unsigned)DAT_05826d10,
                  (int)DAT_07d780a0, (unsigned)DAT_07d78098);
        DbgLogPublic(trace);
    }

    // ── Energy Ball (0x2b) / Hellfire (0x2f): skip packet, just set magic anim ──
    if (skillId == 0x2b || skillId == 0x2f) {
        // SetPlayerMagic(c) — sets attack/cast animation on hero
        FUN_00444a80((int)c);
        return;
    }

    // 004889D0 has a C1:05:10 preamble for every skill except 0x0F, but it
    // is written through the original inline socket/XOR path.  The current
    // Net_SendSmallPacket wrapper is not equivalent for that particular
    // movement-format packet (it diverts the local input flow before Attack).
    // Do not substitute it with FUN_00491C40 either: that is click-to-move.
    // Keep the preamble disabled until that sender is ported byte-for-byte;
    // the direct skill packets below remain on their verified C3 paths.

    // ── Set target position from CharactersClient[MovementSkillTarget] ──
    char* heroEntity = (char*)(DWORD)c;
    int targetIdx = (int)DAT_07d780a0;  // MovementSkillTarget
    if (targetIdx < 0 || targetIdx >= 400) {
        return;
    }
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
    *(float*)((char*)(DWORD)o + 0x24) = angle;

    // ── Switch on skill ID ──
    switch (skillId) {
    case 1: case 2: case 3: case 4:
    case 7: case 0xb: case 0x10: case 0x11:
    case 19: case 20: case 21: case 22: case 23:
    case 43: case 49: case 56:
    {
        // GM name check: compare Hero->ID with "webzen" (DAT_00559d94)
        // Hero->ID at entity offset +0x1C1
        // In original: strlen-based strstr scan. If hero name contains "webzen", skip.
        // This is an anti-impersonation check — GM accounts get special treatment.
        char* heroName = (char*)(entityBase + 0x08 + 0x1C1);  // Hero->ID approximation
        // (Actual Hero ptr is DAT_07abf5d8; for safety just use entity c directly)
        heroName = (char*)((DWORD)c + 0x1C1);
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
        WORD targetKey = *(WORD*)(entityBase + targetIdx * 0x394 + 476);
        if (targetKey == 0xFFFF) {
            DbgLogPublic("WIZ EXIT invalid-target-key");
            break;
        }

        {
            char trace[96];
            wsprintfA(trace, "WIZ SEND19 skill=%u key=%u", (unsigned)skillId,
                      (unsigned)targetKey);
            DbgLogPublic(trace);
        }

        // Real server recv:
        //   PMSG_SKILL_ATTACK_RECV { PBMSG_HEAD // C3:19, BYTE skill, BYTE index[2] }
        SendSkillPacket19_Local((BYTE)skillId, targetKey);
        break;
    }

    case 0xd:
    case 0x34:
    {
        // GM name check: compare Hero->ID with "webzen" (DAT_00559d9c)
        char* heroName = (char*)((DWORD)c + 0x1C1);
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

        // Blast (13) targets the cached world point set from MovementSkillTarget
        // at the beginning of sub_4889D0. Penetration (52) remains centered on
        // the hero's grid square.
        BYTE gridX;
        BYTE gridY;
        if (skillId == 0x0d) {
            gridX = (BYTE)(int)(*(float*)(heroEntity + 788) * 0.01f);
            gridY = (BYTE)(int)(*(float*)(heroEntity + 792) * 0.01f);
        } else {
            gridX = (BYTE)*(DWORD*)(heroEntity + 0x388);
            gridY = (BYTE)*(DWORD*)(heroEntity + 0x38C);
        }

        WORD targetKey = *(WORD*)(entityBase + targetIdx * 0x394 + 476);
        if (targetKey == 0xFFFF) {
            break;
        }
        // PMSG_DURATION_SKILL_ATTACK uses a 0..255 facing byte. The 97k
        // Blast sender leaves distance and angle at zero.
        BYTE dir = (BYTE)(int)(angle * (256.0f / 360.0f));

        // Real server recv:
        //   PMSG_DURATION_SKILL_ATTACK_RECV { C3:1E, skill, x, y, dir, dis, angle, index[2] }
        SendSkillPacket1E_Local((BYTE)skillId, gridX, gridY, dir, 0, 0, targetKey);
        break;
    }

    default:
        // Skills 5,6,8,9,0xa,0xc,0xe,0x10, etc.: no packet sent, fall through
        // Ghidra: goto switchD_00489006_caseD_5 (exit without packet)
        return;
    }

    // SetPlayerMagic(c) — sets cast animation on hero entity
    // Ghidra: SetPlayerMagic @ 0x00444a80
    // If entity type != 0x186: alternate between action 3 and 4 based on combo counter % 3
    // If entity type == 0x186: SetAttackSpeed, then class-specific action
    FUN_00444a80((int)c);
}

// ── Stubs needed by SkillElf ──────────────────────────────────────────────────

static BYTE* GetSkillRecordShadow_Local(int iType)
{
    if (iType < 0 || iType >= 64)
        return nullptr;
    if (DAT_07cf1ff8 != 0)
        return (BYTE*)(uintptr_t)DAT_07cf1ff8 + iType * 0x28;
    return nullptr;
}

static BYTE* GetSkillRecordBase_Local(int iType)
{
    if (iType < 0 || iType >= 64)
        return nullptr;
    if (DAT_07d29d20 != 0)
        return (BYTE*)(uintptr_t)DAT_07d29d20 + iType * 0x28;
    return nullptr;
}

static void SendSkillPacket19_Local(BYTE skillId, WORD targetKey)
{
    BYTE pktBuf[6];
    pktBuf[0] = 0xC1;
    pktBuf[1] = 6;
    pktBuf[2] = 0x19;
    pktBuf[3] = skillId;
    pktBuf[4] = (BYTE)(targetKey >> 8);
    pktBuf[5] = (BYTE)(targetKey & 0xFF);
    {
        char trace[96];
        wsprintfA(trace, "SKILL SEND C1:19 skill=%u targetKey=%u",
                  (unsigned)skillId, (unsigned)targetKey);
        DbgLogPublic(trace);
    }
    SendMuEmuEncryptedPacket(pktBuf, 6);
}

static void SendSkillPacket1E_Local(BYTE skillId, BYTE gridX, BYTE gridY, BYTE dir, BYTE dist, BYTE angle, WORD targetKey)
{
    BYTE pktBuf[11];
    pktBuf[0] = 0xC1;
    pktBuf[1] = 11;
    pktBuf[2] = 0x1E;
    pktBuf[3] = skillId;
    pktBuf[4] = gridX;
    pktBuf[5] = gridY;
    pktBuf[6] = dir;
    pktBuf[7] = dist;
    pktBuf[8] = angle;
    pktBuf[9] = (BYTE)(targetKey >> 8);
    pktBuf[10] = (BYTE)(targetKey & 0xFF);
    {
        char trace[128];
        wsprintfA(trace,
                  "SKILL SEND C1:1E skill=%u xy=(%u,%u) dir=%u dis=%u angle=%u targetKey=%u",
                  (unsigned)skillId, (unsigned)gridX, (unsigned)gridY,
                  (unsigned)dir, (unsigned)dist, (unsigned)angle,
                  (unsigned)targetKey);
        DbgLogPublic(trace);
    }
    SendMuEmuEncryptedPacket(pktBuf, 11);
}

// GetSkillInformation @ 0x0047E7A0 — reads skill table entry for given type/level.
// Outputs mana cost, distance, and AG (SkillMana) cost via out-pointers.
// Real implementation uses the compact 0x28-byte skill record table.
void __cdecl GetSkillInformation(int iType, int iLevel, char* lpszName, int* piMana, int* piDistance, int* piSkillMana) {
    (void)iLevel;
    BYTE* baseEntry = GetSkillRecordBase_Local(iType);
    BYTE* statEntry = GetSkillRecordShadow_Local(iType);
    if (statEntry == nullptr)
        statEntry = baseEntry;
    if (baseEntry == nullptr && statEntry == nullptr) {
        if (lpszName) lpszName[0] = '\0';
        if (piMana) *piMana = 0;
        if (piDistance) *piDistance = 0;
        if (piSkillMana) *piSkillMana = 0;
        return;
    }

    if (lpszName != NULL) {
        const char* srcName = nullptr;
        if (baseEntry && ((const char*)baseEntry)[0] != '\0')
            srcName = (const char*)baseEntry;
        else if (statEntry && ((const char*)statEntry)[0] != '\0')
            srcName = (const char*)statEntry;
        else
            srcName = "";
        strncpy_s(lpszName, 256, srcName, 31);
        lpszName[31] = '\0';
        for (char* p = lpszName; *p; ++p) {
            unsigned char ch = (unsigned char)*p;
            if (ch < 32)
                *p = ' ';
        }
    }
    if (piMana != NULL) {
        int v = (int)*(WORD*)(statEntry + 0x22);
        *piMana = (v >= 0 && v <= 5000) ? v : 0;
    }
    if (piDistance != NULL) {
        int v = (int)*(BYTE*)(statEntry + 0x27);
        *piDistance = (v >= 0 && v <= 50) ? v : 0;
    }
    if (piSkillMana != NULL) {
        int v = (int)*(BYTE*)(statEntry + 0x26);
        *piSkillMana = (v >= 0 && v <= 255) ? v : 0;
    }
}


// SkillElf @ 0x0048BD70 (~1247 lines)
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
// IDA: FUN_0048A180 @ 0x0048A180 — SkillElf. ~60% of the original 1247 lines is anti-tamper hash table operations
// (FUN_00403f80, FUN_004041e0, FUN_00404280, FUN_00404330, FUN_00404370,
//  FUN_00404400) and XOR key init + dead forward/reverse loops — all skipped.
bool __stdcall SkillElf_stub(DWORD c, DWORD pItem) {
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

    BYTE skillCount = *(BYTE*)(charAttrBase + 86);
    if (skillCount == 0) {
        return false;
    }
    BYTE selectedSkillId = ResolveQueuedSkillId97k();
    if ((int)DAT_07d780a0 < 0 || (int)DAT_07d780a0 >= 400) {
        return false;
    }

    // CharacterAttribute->Mana at offset 0x1D (WORD) — Ghidra reads high byte via +1
    // Actually Ghidra: *(ushort *)((int)&CharacterAttribute->Mana + 1) — reads misaligned
    // CharacterAttribute->Mana is at 0x1D, so +1 = 0x1E. This reads bytes [0x1E..0x1F]
    // as a ushort — which is MaxLife low byte + MaxLife high byte. But more likely
    // the Ghidra struct layout: Mana at 0x1D (WORD), so &Mana+1 = 0x1E.
    // This is actually reading Mana as big-endian or the full Mana value.
    // Let's just read the WORD at offset 0x1D for current Mana.
    WORD currentMana = *(WORD*)(charAttrBase + 0x1D);
    // CharacterAttribute->SkillMana at offset 0x23 (WORD)
    // Ghidra: *(ushort *)((int)&CharacterAttribute->SkillMana + 1) => offset 0x24
    WORD currentAG = *(WORD*)(charAttrBase + 0x23);

    for (int i = 0; i < (int)skillCount; i++) {
        BYTE skillId = *(BYTE*)(charAttrBase + 87 + i);

        // Only process the skill that matches the hero's currently selected skill
        if (selectedSkillId != skillId) continue;

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
        BYTE gridX = (BYTE)*(DWORD*)(heroEntity + 0x388);
        BYTE gridY = (BYTE)*(DWORD*)(heroEntity + 0x38C);

        // Skill type check: 0x18 = ranged arrow skill
        if (skillId == 0x18) {
            // Check if arrows are equipped
            char hasArrow = Combat_CheckArrowRequirement();  // CheckArrow
            if (hasArrow == '\0') continue;  // no arrows equipped

            BYTE* skillEntry = GetSkillRecordShadow_Local((int)skillId);
            if (skillEntry == nullptr)
                skillEntry = GetSkillRecordBase_Local((int)skillId);
            if (skillEntry == nullptr)
                continue;

            // anti-tamper hash table — skipped (encrypt skill entry)

            // Compact 97k record: distance is stored at +0x27.
            BYTE skillDistance = *(BYTE*)(skillEntry + 0x27);

            // Get hero position
            float heroPosX = *(float*)(heroEntity + 0x10);  // Object.Position[0]
            float heroPosY = *(float*)(heroEntity + 0x14);  // Object.Position[1]

            // Use the current runtime target entity position, not the stale
            // global ground-target coords. This keeps ranged-elf range checks
            // aligned with the same target slot used by the packet send.
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

            // 97k runtime uses the same move sender path before the actual
            // skill packet so target/facing/path state stay aligned with the
            // rest of combat helpers.
            Combat_SendMovePathPacket((int)heroEntity, (int)heroEntity);

            // MuEmu's 0.97k continuation patch for Triple Shot uses the
            // 256-step facing byte, the packed target delta, and an opposite
            // angle.  The original routine at 0048BD70 computes the same
            // facing value with angle * 0.71111113.
            BYTE dir = (BYTE)(int)(angle * (256.0f / 360.0f));
            BYTE destination = PackDurationDestination_Local(
                gridX, gridY,
                (int)*(unsigned char*)(heroEntity + 0x306),
                (int)*(unsigned char*)(heroEntity + 0x307));
            BYTE oppositeAngle = (BYTE)(int)((angle + 180.0f) * (256.0f / 360.0f));

            // Align to the same server recv struct family used by the wizard path:
            //   PMSG_DURATION_SKILL_ATTACK_RECV { C3:1E, skill, x, y, dir, dis, angle, index[2] }
            SendSkillPacket1E_Local(skillId, gridX, gridY, dir, destination, oppositeAngle, targetKey);

            // Set player attack animation
            // SetPlayerAttack(hero) — Ghidra shows 1-arg; use 4-arg decl with dummies
            FUN_00444410((int)(DWORD)heroEntity, 0, 0, 0);

            // If object type is not 0x186 (special entity), create arrow projectiles
            WORD objType = *(WORD*)(heroEntity + 0x02);  // Object.Type at offset +0x02
            if (objType != 0x186) {
                // We are already iterating the real runtime skill list and matched
                // the selected skill above, so the arrow effect should use the same
                // runtime slot instead of rescanning a stale/corrupted skill block.
                int skillIndex = i;

                // Spawn arrow visual effect
                CreateArrows_stub((DWORD)heroEntity, (DWORD)(heroEntity),
                                  (DWORD)0, (WORD)skillIndex, 1, 0);
            }
            result = true;
            continue;
        }

        DAT_05826d10 = (DWORD)skillId;

        // Keep elf/support on the same pre-move runtime path as warrior/wizard.
        Combat_SendMovePathPacket((int)heroEntity, (int)heroEntity);

        // Heal (26), Greater Defense (27) and Greater Damage (28) are
        // targeted skills.  MuEmu receives them through C3:19; C3:1E is
        // reserved for duration/area attacks and rejects their normal path.
        SendSkillPacket19_Local(skillId, targetKey);

        FUN_00444a80((int)(DWORD)heroEntity);
        result = true;
    }

    // anti-tamper hash table — skipped (decrypt CharacterMachine after access)
    return result;
}


// FUN_0043d670 @ 0x0043D670 (~188 lines) — hash table: read 4-byte encrypted value
// FUN_0043d670 removed — __thiscall hash table read, not needed as free function
