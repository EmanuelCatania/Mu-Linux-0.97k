// Entity_MoveClient.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// MoveCharactersClient @ 0x00455010 (~169 lines) — SUMMARY STUB
// Per-frame entity update. Clears terrain walk flags, then for each alive character:
// decrypt movement via hash table, tick movement, re-encrypt, update cached waypoints.
void __stdcall MoveCharactersClient_stub(void) {
    // 0x00455010 — Per-frame entity update
    // Phase 1: Clear bit 2 from all terrain walk flags (0x10000 tiles)
    // Phase 2: For each alive entity, decrypt movement waypoints via hash table,
    //          compute terrain index, set walk-occupied bit
    // Phase 3: Call MoveCharacterClient for each entity, then MoveBlurs

    // Phase 1: Clear walk-occupied bit from terrain
    char* terrainWall = (char*)&DAT_0838bc70;
    for (int i = 0; i < 0x10000; i++) {
        if ((terrainWall[i] & 2) == 2) {
            terrainWall[i] = terrainWall[i] - 2;
        }
    }

    // Phase 2: Per-entity terrain flag update
    int offset = 0;
    char* entBase = (char*)(uintptr_t)DAT_07abf5d0;
    do {
        char* ent = entBase + offset;
        // entity alive check: ent[0] != 0, ent[0x2fd] == 0, ent[0x84] != 8
        if (ent[0] != '\0' && ent[0x2fd] == '\0' && ent[0x84] != '\x08') {
            // anti-tamper hash table — skipped (encrypt ent+0x38c waypoint)
            // anti-tamper hash table — skipped (decrypt ent+0x38c waypoint)

            int val_38c = *(int*)(ent + 0x38c);  // cached_wp_y (audit #10: comment was swapped)

            // anti-tamper hash table — skipped (encrypt ent+0x388 waypoint)
            // anti-tamper hash table — skipped (decrypt ent+0x388 waypoint)

            int val_388 = *(int*)(ent + 0x388);  // cached_wp_x (audit #10: comment was swapped)

            // Compute terrain index from cached waypoints and set walk-occupied bit
            // Call order from asm: FUN_004f6c40(val_388, val_38c)
            int terrainIdx = FUN_004f6c40((unsigned int)val_388, (unsigned int)val_38c);
            terrainWall[terrainIdx] = terrainWall[terrainIdx] | 2;
        }
        offset += 0x394;
    } while (offset < 0x59740);

    // Phase 3: Tick movement for each entity, then blur trails
    offset = 0;
    do {
        FUN_00454fc0((float*)(entBase + offset));
        offset += 0x394;
    } while (offset < 0x59740);

    MoveBlurs();
}

// FUN_0045fae0 @ 0x0045FAE0 (~175 lines) — hash table: read 1-byte encrypted value
unsigned char __cdecl FUN_0045fae0(DWORD ecx, unsigned char* param_1) {
    // 0x0045FAE0 — anti-tamper hash table: read 1-byte encrypted value
    // Original is __thiscall on the hash table object (ecx = this).
    // Looks up param_1 key in hash table, allocates+decrypts a 1-byte node,
    // writes decrypted value to *param_1, then releases the node.
    //
    // Decryption: val = ((stored_byte - 0x23) ^ DAT_00559050) + 0xb9
    //
    // anti-tamper hash table — skipped
    // Since the hash table encrypt/decrypt is anti-tamper obfuscation and not real game logic,
    // this function is a no-op. The byte at *param_1 is left unchanged (already correct
    // in the unprotected build).
    (void)ecx;
    (void)param_1;
    return *param_1;
}
