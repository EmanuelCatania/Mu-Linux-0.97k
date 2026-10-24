// Party_NameMatch.cpp
// IDA: FUN_004afb00 @ 0x004afb00
//
// Party_MatchEntityNames — matches party slot names to live entity indices.
//
// Scans the entity array (base DAT_07abf5d0, stride 0x394) for entries with
// entity_type == 0x186 (player) and pcVar7[0x84] == 1 (active/visible).
// For each party slot starting at DAT_07e11e9c (stride 0x24):
//   - Compares slot name (_Str1) with entity name at pcVar7+0x1c1 via strncmp
//   - On match: writes entity index into *(int*)(_Str1+0x1c), entity pos ptr into *(int*)(_Str1+0x20)
// Second loop: for slots with index < 0 ("unmatched"),
//   - Checks if hero entity name matches → writes -3 (matched hero) or -1 (no match)

#include "stdafx.h"
#include "Party/Party.h"
#include "Entity/EntityName.h"

// IDA: FUN_004afb00
void __cdecl Party_MatchEntityNames(void)
{
    // sub_4AFB00 scans the 400-entry CharactersClient array (0x394 bytes
    // per entry), not the address of the DAT_07abf5d0 pointer variable.
    // The last two predicates are material: a party member that is merely
    // allocated but not on-screen must not receive an overhead HP bar.
    char* entity = (char*)(uintptr_t)DAT_07abf5d0;
    if (!entity)
        return;

    for (int entityIndex = 0; entityIndex < 400; ++entityIndex, entity += 0x394) {
        if (AsObject(entity)->Type != 390 || *(char *)&AsObject(entity)->Kind != 1 || !*(char *)&AsObject(entity)->Live ||
            !*(char *)&AsObject(entity)->Visible || AsObject(entity)->Alpha <= 0.0f || entity[765]) {
            continue;
        }

        for (int partyIndex = 0; partyIndex < PartyNumber; ++partyIndex) {
            char* slot = (char*)Party + partyIndex * 36;
            if (*(int*)(slot + 28) != -2)
                continue;

            const char* slotName = Party_NameOf(slot);
            const size_t nameLength = strlen(EntityName::Get(entity)) ? strlen(EntityName::Get(entity)) : 1;
            const size_t compareLength = strlen(slotName) <= nameLength
                ? nameLength : strlen(slotName);
            *(int*)(slot + 32) = 0;
            if (strncmp(slotName, EntityName::Get(entity), compareLength) == 0) {
                *(int*)(slot + 28) = entityIndex;
                *(int*)(slot + 32) = *(int*)(entity + 120);
                break;
            }
        }
    }

    // The local hero is intentionally represented by -3.  RenderPartyHP
    // ignores it, while the party panel uses the sentinel to distinguish the
    // hero from an entity that has not entered the viewport yet.
    for (int partyIndex = 0; partyIndex < PartyNumber; ++partyIndex) {
        char* slot = (char*)Party + partyIndex * 36;
        if (*(int*)(slot + 28) >= 0)
            continue;

        const char* heroName = Hero ? EntityName::Get(Hero) : "";
        const size_t heroLength = strlen(heroName) ? strlen(heroName) : 1;
        const char* slotName = Party_NameOf(slot);
        const size_t compareLength = strlen(slotName) <= heroLength
            ? heroLength : strlen(slotName);
        *(int*)(slot + 32) = 0;
        *(int*)(slot + 28) = (Hero && strncmp(slot, heroName, compareLength) == 0)
            ? -3 : -1;
    }
}
