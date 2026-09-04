// Item_Equip.cpp
// IDA: Item_Equip @ 0x00483B30 (7193 bytes) — char __cdecl Item_Equip(DWORD o, int a2).
// El nombre historico engania: NO es una operacion de UI de inventario. Es el
// ejecutor de skills que Attack (0x0049CBF0 L1426) invoca por cada mano
// equipada, y devuelve si consumio el tick de ataque.
//   o  (nuestro `character`) = CHARACTER* del heroe.
//   a2 (nuestro `object`)    = ITEM* equipado (CharacterMachine + 536 + 68*mano).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"
#include "Item_Equip.h"

// IDA L239: v13 = *(unsigned __int8 *)(dword_7D7809C + CharacterAttribute + 87).
// El indice es dword_7D7809C (el slot que Attack acaba de sembrar en L1425 con
// *(BYTE *)(Hero + 913)), no Hero+913 releido. Coinciden en el camino de Attack,
// pero UseSkillWarrior usa el mismo global y conviene mantenerlos atados.
static BYTE ItemEquip_SelectedSkill()            // IDA: v13
{
    BYTE* const attributes = (BYTE*)(uintptr_t)DAT_07cf1ff4;   // IDA: CharacterAttribute
    if (!attributes)
        return 0;
    return attributes[(BYTE)DAT_07d7809c + 87];
}

// IDA: Item_Equip @ 0x483B30, L284-505 and LABEL_149-216. Both the direct
// 43/47/49 path and the per-item Special[] path use the same C1:05:26 request
// when mana is insufficient. Item_Equip owns its own return/continue behavior.
static void ItemEquip_UseManaScroll97k(int inventorySlot /* IDA: sub_482BE0(3) */)
{
    if (inventorySlot == -1)
        return;
    if (WarehouseOpened || TradeOpened) {
        UIChatLogWindow_AddText("ERROR", GlobalText[474], 2); // IDA: GlobalText[474]
        return;
    }
    if (EnableUse > 0)
        return;

    EnableUse = 10;
    const BYTE packet[5] = { 0xC1, 5, 0x26, (BYTE)(inventorySlot + 12), 0 };
    Net_SendSmallPacket(packet, sizeof(packet));

    // IDA: *(&OffsetInventoryItems.Type + 34 * inventorySlot). Type is a
    // WORD, hence the 34-WORD = 68-byte ITEM stride.
    const WORD itemType = ((ITEM*)OffsetInventoryItems)[inventorySlot].Type;
    if (itemType == 448)
        PlayBuffer(33, 0, 0);
    else if (itemType >= 449 && itemType <= 457)
        PlayBuffer(32, 0, 0);
}

// IDA: sub_483780 @ 0x00483780 (936 bytes)
//   char __cdecl sub_483780(int sy, DWORD a2, int a3, int a4)
//   sy = CHARACTER*, a2 = OBJECT*, a3 = ITEM* (no se usa en el cuerpo), a4 = skill.
// Nombres: skill = a4, target = CharactersClient[SelectedCharacter],
// routeReach = SkillAttribute[40*a4+38]*1.2 (v12 antes del override), reach = v12.
static char ItemEquip_UseSelectedTargetSkill(DWORD character /* IDA: sy */,
                                             DWORD object    /* IDA: a2 */,
                                             BYTE  skill     /* IDA: a4 */)
{
    if (SelectedCharacter == -1 || !character || !object)
        return 0;                                     // IDA L38-41

    BYTE* const caster = (BYTE*)(uintptr_t)character;
    BYTE* const target = (BYTE*)(uintptr_t)CharactersClient + 916 * (int)SelectedCharacter;
    // IDA L44-49: sub_483780 es duenio del cast encolado; a diferencia del camino
    // de rango generico de Attack limpia el flag pendiente antes de anotar el slot.
    DAT_07d78098 = 0;                                 // IDA: dword_7D78098 = 0
    DAT_07d7809c = skill;                             // IDA: dword_7D7809C = a4
    DAT_07d780a0 = (DWORD)SelectedCharacter;          // IDA: MovementSkillTarget

    const int targetX = (int)(*(float*)(target + 16) * 0.01f);   // IDA: TargetX
    const int targetY = (int)(*(float*)(target + 20) * 0.01f);   // IDA: TargetY (v5)
    DAT_07e016c0 = (DWORD)targetX;
    DAT_07e016c4 = (DWORD)targetY;
    const float dx = *(float*)(object + 16) - (targetX * 100.0f + 50.0f);  // IDA: v14
    const float dy = *(float*)(object + 20) - (targetY * 100.0f + 50.0f);  // IDA: v13
    BYTE* const skillInfo = (BYTE*)SkillAttribute.Raw + skill * 0x28;
    const float routeReach = skillInfo[0x26] * 1.2f;  // IDA L98-99: v33 * 1.2
    float reach = routeReach;                         // IDA: v12
    if (World >= 11 && World <= 16 && skill >= 19 && skill <= 23)
        reach = 1.8f;                                 // IDA L100-103

    if (dx * dx + dy * dy > reach * reach * 10000.0f) {   // IDA L106: v12*100 < sqrt(...)
        // IDA L145-146: el pathfind usa routeReach, NO el override de 1.8.
        if (!Path_FindRoute(*(int*)(character + 904), *(int*)(character + 908),
                            targetX, targetY, caster + 852, routeReach))
            return 0;                                 // IDA: goto LABEL_33 con v30 = 0
        caster[748] = 1;                              // IDA L150
        caster[749] = 5;                              // IDA L151
        return 1;
    }

    Combat_UseWarriorSkill((int)character, (int)object);  // IDA L155: UseSkillWarrior
    return 1;                                         // IDA L157: v30 = 1
}

// IDA L551-1172 (LABEL_25): recorre la lista de opciones especiales del ITEM
// equipado y solo despacha el skill si el arma efectivamente lo concede.
//   *(BYTE *)(a2 + 36)      = ITEM::SpecialNum
//   *(BYTE *)(a2 + 37 + i)  = ITEM::Special[i]
// ItemConvert (0x0047B910 L342-430) es quien mete ahi los ids 18..24, 49 y 56
// cuando el item trae la opcion Skill (Attribute1 & 0x80).
static char ItemEquip_DispatchFromItemSkillList(DWORD character, DWORD object, BYTE* item)
{
    BYTE* const attributes = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (!attributes || !Hero || !item)
        return 0;

    const BYTE selected = attributes[(BYTE)*(BYTE*)(Hero + 913) + 87];   // IDA L558
    const int  count    = (int)item[36];                                 // IDA: SpecialNum
    char result = 0;                                                     // IDA: v136

    for (int i = 0; i < count; ++i) {                 // IDA L556 / L1167-1172
        const BYTE skill = item[37 + i];              // IDA: v51[v50 + 37]
        if (skill != selected)
            continue;                                 // IDA L558

        // IDA L560-561: el 4o argumento se reusa como coste de mana y se compara
        // contra CharacterAttribute+30.
        int manaCost = 0;
        GetSkillInformation((int)skill, 1, 0, &manaCost, 0, 0);
        if ((int)*(unsigned short*)(attributes + 30) < manaCost) {
            ItemEquip_UseManaScroll97k(FUN_00482be0(3)); // IDA L909-1161
            continue;
        }

        switch (skill) {
        case 0x12: {                                  // IDA L565: skill 18
            // El objetivo es la key del propio heroe, no SelectedCharacter.
            *(BYTE*)(uintptr_t)(character + 748) = 0;
            if (*(WORD*)(uintptr_t)(character + 2) == 390)
                FUN_0043e820((int)character, 92);     // IDA: SetAction(v4, 92)
            else
                FUN_00444410((int)character, 0, 0, 0); // IDA: SetPlayerAttack(v4)

            const DWORD now = GetTickCount();          // IDA L599-603
            if ((DWORD)(now - DAT_05826cf4) <= 300) { result = 1; break; }
            DAT_05826cf4 = now;
            const WORD heroKey = *(WORD*)(Hero + 476); // IDA: *(_WORD *)(Hero + 476)
            BYTE packet[6] = { 0xC1, 6, 0x19, 18,
                               (BYTE)(heroKey >> 8), (BYTE)heroKey };
            Net_SendSmallPacket(packet, sizeof(packet));
            result = 1;                                // IDA L893: v136 = 1
            break;
        }
        case 0x13: case 0x14: case 0x15:              // IDA L895-904: 19..23 y 49
        case 0x16: case 0x17: case 0x31:
            if (FUN_00483160())                        // IDA: CheckAttack()
                result = ItemEquip_UseSelectedTargetSkill(character, object, skill);
            break;
        default:                                       // IDA L906: goto LABEL_217
            break;
        }
    }
    return result;
}

char __cdecl Item_Equip(DWORD character /* IDA: o */, DWORD object /* IDA: a2 */)
{
    if (!character || !object)
        return 0;

    // OJO con la firma historica: el binario llama Item_Equip(c, item) y despues
    // usa o (= nuestro `character`) tanto de CHARACTER* como de OBJECT*, y a2
    // (= nuestro `object`) como el ITEM equipado. Los nombres de los parametros
    // quedaron invertidos en el port; se conservan por compatibilidad de callers.
    BYTE* const caster = (BYTE*)(uintptr_t)character;
    BYTE* const item   = (BYTE*)(uintptr_t)object;

    const WORD type   = *(WORD*)(caster + 2);          // IDA: *(_WORD *)(o + 2)
    const BYTE action = caster[261];                   // IDA: v5

    // IDA L201-212: rechaza las animaciones de arranque de ataque.
    if (type == 390) {
        if (action == 92 || (action >= 56 && action <= 59) || action == 81)
            return 0;
    } else if (action >= 3 && action <= 4) {
        return 0;
    }

    const BYTE skill = ItemEquip_SelectedSkill();      // IDA: v13
    bool useDirectPath = true;                         // false == goto LABEL_25

    // IDA L251-283.
    if (skill == 47) {
        const WORD mount = *(WORD*)(Hero + 696);       // IDA: v17
        if (mount != 818 && mount != 819) {
            // IDA L256: sin montura NO devuelve 0 — cae en LABEL_25 y el skill
            // puede salir igual por la lista de opciones del arma. El port
            // anterior cortaba aca con `return 0`.
            useDirectPath = false;
        } else {
            if (type != 390)
                return 0;
            const WORD weapon = *(WORD*)(Hero + 624);  // IDA: v26
            if (weapon == 0xFFFF || weapon < 496 || weapon >= 528)
                return 0;
        }
    } else if (skill != 49) {
        if (skill != 43) {
            useDirectPath = false;                     // IDA L272: goto LABEL_25
        } else {
            if (type != 390)
                return 0;
            const WORD weapon = *(WORD*)(Hero + 624);  // IDA: v21
            if (weapon == 0xFFFF || (weapon >= 560 && weapon < 592))
                return 0;
        }
    }

    if (useDirectPath) {
        // IDA L284-522.
        BYTE* const attributes = (BYTE*)(uintptr_t)DAT_07cf1ff4;
        int piMana = 0, piSkillMana = 0;
        GetSkillInformation((int)skill, 1, 0, &piMana, 0, &piSkillMana);
        if (!attributes)
            return 0;
        if ((int)*(unsigned short*)(attributes + 30) < piMana) {
            ItemEquip_UseManaScroll97k(FUN_00482be0(3)); // IDA L285-505
            return 0;
        }
        if (piSkillMana > (int)*(unsigned short*)(attributes + 36))
            return 0;                                  // IDA L516-518
        if (FUN_00483160())                            // IDA L520: CheckAttack()
            return ItemEquip_UseSelectedTargetSkill(character, character, skill);
        // IDA: si CheckAttack falla NO retorna — sigue por LABEL_25.
    }

    return ItemEquip_DispatchFromItemSkillList(character, character, item);
}
