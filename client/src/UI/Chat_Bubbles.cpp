// Chat_Bubbles.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" const char* Guild_GetMarkName(int row);

// FUN_0047fed0 @ 0x0047FED0 (~230 lines) — whisper name validator
// Encrypts CharacterMachine via MAIN_HASH_CLASS, reads CharacterAttribute->Level,
// then decrypts. Compares a phantom param (unaff_retaddr = whisper target name)
// against the WhisperRegistID table (28 entries, stride 10).
// Returns 1 if name is already registered or level check passes; 0 + AddText if not found.
//
// ~80% of the Ghidra output is anti-tamper hash table encrypt/decrypt — skipped.
// Phantom stack param (unaff_retaddr) carries the whisper target name string.
// Since we can't access unaff_retaddr in C, this stub checks the WhisperRegistID table
// using the __stdcall convention (no real params visible).
int __stdcall FUN_0047fed0(void) {
    // anti-tamper hash table — skipped (encrypt CharacterMachine)

    // Read CharacterAttribute->Level (byte at offset +1 from Level field)
    // Ghidra: local_8 = (uint)*(ushort *)((int)&CharacterAttribute->Level + 1)
    // This is the character level used for the comparison iStack_4 <= iStack_c.
    // Since we skip anti-tamper, the level comparison is unreliable without
    // the phantom param. Default to "pass" (return 1).

    // anti-tamper hash table — skipped (decrypt CharacterMachine)

    // The original then checks if the whisper target (phantom param) is in
    // WhisperRegistID[0..27] (stride 10 bytes each). If found, returns 1.
    // If not found, calls AddText(&DAT_07e11dd4, &DAT_07d4ce78, 1) and returns 0.
    //
    // Without the phantom param we cannot implement the name lookup.
    // Return 1 (always pass) to match normal runtime behavior.
    return 1;
}

// FUN_00481a40 @ 0x00481A40 (~64 lines) — assign chat text to chat entry struct
// Long text (>19 chars): CutText to split into 2 lines at offsets 0x2c and 300.
// Short text: direct memcpy to offset 0x2c.
// param_3==1: system message via vtable call on DAT_055c9ff0.
void __cdecl FUN_00481a40(int param_1, char* param_2, int param_3) {
    // 0x00481A40 — Assign chat text to chat entry struct
    // param_1 = chat entry base pointer
    // param_2 = text string
    // param_3 = 0: normal chat, 1: system message via vtable

    int Length = (int)strlen(param_2);

    char* displayEnd;
    if (param_3 == 0) {
        displayEnd = (char*)((unsigned int)Length + (unsigned int)Length + 0xa0);
    } else if (param_3 == 1) {
        // System message: dispatch slot 28 (vtable +0x70) = ChatLB_AddText
        // (__fastcall this + 4 args).  Previous cdecl form left ECX = junk
        // and corrupted the chat list.
        if (DAT_055c9ff0 && *(int*)DAT_055c9ff0) {
            DWORD* obj = (DWORD*)DAT_055c9ff0;
            void** vt  = (void**)*obj;
            typedef void (__fastcall *FnAddText)(DWORD*, int /*edx*/,
                                                 const char*, const char*,
                                                 int, int);
            ((FnAddText)vt[28])(obj, 0, (const char*)param_1,
                                (const char*)param_2, 3, 0);
        }
        displayEnd = (char*)0x3e8;
    } else {
        displayEnd = param_2;
    }

    if (Length > 0x13) {
        // Long text (>19 chars): split into 2 lines
        FUN_0047fe30(param_2, param_1 + 0x2c, (void*)(param_1 + 300), Length);
        *(char**)(param_1 + 0x22c) = displayEnd;
        *(char**)(param_1 + 0x230) = displayEnd;
        return;
    }

    // Short text: direct memcpy to offset 0x2c
    memcpy((void*)(param_1 + 0x2c), param_2, Length + 1);
    *(char**)(param_1 + 0x22c) = displayEnd;
}

// CreateChat @ 0x00481BA0 (~396 lines) — creates chat bubble above entity
// Chat bubble pool: base DAT_07e016f8, stride 0x254, ~96 slots.
// Each slot has: ID (+0x00), guild (+0x18), color (+0x24), team (+0x25),
//   timer1 (+0x28), text1 (+0x2c), text2 (+0x12c), disp1 (+0x22c), disp2 (+0x230), owner (+0x234).
// Two paths: (1) find existing slot for same owner and update, (2) find empty slot and create.
void __cdecl FUN_00481ba0(char* ID, char* Text, DWORD entity, int Flag, int SetColor) {
    // Guard: entity must be alive and visible
    // BUG-FIX 2026-07-19: offsets de entidad equivocados (mismo problema que
    // AssignChat). IDA CreateChat @0x481BA0:
    //     if ( *(_BYTE *)Owner && *(_BYTE *)(Owner + 352) )
    //     v5 = *(unsigned __int8 *)(Owner + 746);          // PK
    //     if ( *(_BYTE *)(Owner + 132) == 4 ) v5 = 0;      // Kind
    if (*(BYTE *)entity == 0) return;              // Object.Live   → +0
    if (*(BYTE *)(entity + 352) == 0) return;      // Object.Visible → +352 (0x160)

    // Resolve SetColor: -1 means use PK color, but Kind==4 forces 0
    if (SetColor == -1) {
        SetColor = (int)*(BYTE *)(entity + 746);    // Owner->PK      → +746 (0x2EA)
        if (*(BYTE *)(entity + 132) == 4) {         // Object.Kind==4 → +132 (0x84)
            SetColor = 0;
        }
    }

    // BUG-FIX 2026-07-19 (CRASH 0xC0000005 en FUN_00481ba0+0x86): el bound era
    // la dirección LITERAL del binario original (`POOL_END = 0x7e0ffc8`) y la
    // base era `&DAT_07e016f8`, que estaba declarado como un char de 1 BYTE.
    // El walk se paseaba por memoria ajena hasta reventar en
    // `*(DWORD*)(slot + 0x234)`. Hasta ahora no se notaba porque AssignChat
    // nunca matcheaba (offsets de entidad mal) → CreateChat era código muerto.
    // Pool real: base 0x7E016F8, stride 596, fin 0x7E0FFC8 → (0xE8D0)/596 = 100.
    char *pool_base = DAT_07e016f8;
    const int STRIDE = 0x254;
    const int POOL_SLOTS = 100;

    // ── Path 1: Find existing slot owned by this entity ──
    char *slot = pool_base;
    for (int si = 0; si < POOL_SLOTS; ++si, slot += STRIDE) {
        if (*(DWORD *)(slot + 0x234) == entity)
            goto found_existing;
    }

    // ── Path 2: Find empty slot (both timers <= 0) ──
    slot = pool_base;
    for (int si = 0; si < POOL_SLOTS; ++si, slot += STRIDE) {
        if (*(int *)(slot + 0x28) <= 0 && *(int *)(slot + 0x22c) <= 0)
            goto found_new;
    }
    return;  // no free slot

found_new:
    *(DWORD *)(slot + 0x234) = entity;
    // Copy ID
    strcpy(slot, ID);
    // Set color and guild info
    *(char *)(slot + 0x24) = (char)SetColor;
    *(char *)(slot + 0x25) = '\0';

    {
        short guildIdx = *(short *)(entity + 474);  // GuildMarkIndex — IDA +474 (0x1DA), no 0x1d8
        if (guildIdx < 0) {
            *(char *)(slot + 0x18) = '\0';
        } else {
            *(char *)(slot + 0x25) = *(BYTE *)(entity + 745);  // GuildTeam
            // Build "[GuildName]" string at +0x18
            strcpy(slot + 0x18, &DAT_00559d60);  // prefix "["
            // Append guild mark name (stride 0x50 per guild, from GuildMark global)
            // GuildMark not yet declared — leave guild bracket empty for now
            // strcat(slot + 0x18, &GuildMark + guildIdx * 0x50);
            strcat(slot + 0x18, Guild_GetMarkName(guildIdx));
            // Append suffix
            char *end = slot + 0x18 + strlen(slot + 0x18);
            *(short *)end = DAT_00559d64;
            end[2] = DAT_00559d66;
        }
    }

    // Set text: empty text → timer=100, return
    if (strlen(Text) == 0) {
        *(int *)(slot + 0x28) = 100;
        return;
    }

    // Assign text using FUN_00481a40 logic
    FUN_00481a40((int)slot, Text, Flag);
    return;

found_existing:
    // PORT FIEL a IDA CreateChat @0x481BA0 found-existing path (2026-07-25):
    // Antes hacíamos el shift text1→text2, re-seteábamos owner y limpiábamos
    // text1 INCONDICIONALMENTE.  Pero Target_Render llama esto cada frame con
    // Text="" mientras hacés hover sobre un NPC → el manoseo per-frame de las
    // líneas hacía que el nombre se dibujara solapado varias veces.
    // IDA: para Text vacío hace SOLO `v6[10]=10` (refresca timer); el shift +
    // owner + set-text SOLO ocurren cuando Text NO está vacío.

    // Copy ID (name) — siempre
    strcpy(slot, ID);
    // Set color and guild info — siempre
    *(char *)(slot + 0x24) = (char)SetColor;
    *(char *)(slot + 0x25) = '\0';

    {
        short guildIdx2 = *(short *)(entity + 474);
        if (guildIdx2 < 0) {
            *(char *)(slot + 0x18) = '\0';
        } else {
            *(char *)(slot + 0x25) = *(BYTE *)(entity + 745);
            strcpy(slot + 0x18, &DAT_00559d60);
            strcat(slot + 0x18, Guild_GetMarkName(guildIdx2));
            char *end2 = slot + 0x18 + strlen(slot + 0x18);
            *(short *)end2 = DAT_00559d64;
            end2[2] = DAT_00559d66;
        }
    }

    if (strlen(Text) != 0) {
        // Texto nuevo: shift la línea actual a "previa", set owner, asignar texto.
        if (*(int *)(slot + 0x22c) > 0) {                     // disp1 > 0
            strcpy(slot + 0x12c, slot + 0x2c);                // text1 → text2
            *(int *)(slot + 0x230) = *(int *)(slot + 0x22c);  // disp1 → disp2
        }
        *(DWORD *)(slot + 0x234) = entity;
        FUN_00481a40((int)slot, Text, Flag);
    } else {
        // Texto vacío (hover de nombre NPC/target): SOLO refrescar el timer.
        *(int *)(slot + 0x28) = 10;
    }
}

// AssignChat @ 0x00482090 (~78 lines) — finds character by ID, creates chat bubble
// Pass 1: search Kind==1 (players). Pass 2: search Kind==2 (NPCs).
// On match: CreateChat(ID, Text, character, Flag, -1).
void __cdecl AssignChat_stub(char* ID, char* Text, int Flag) {
    // 0x00482090 — Find character by ID, create chat bubble
    //
    // BUG-FIX 2026-07-19 (LA BURBUJA NUNCA APARECÍA): los offsets estaban mal.
    // IDA AssignChat @0x482090:
    //     if ( *(_BYTE *)v4 && *(_BYTE *)(v4 + 132) == 1 )
    //   → activo en **+0** (byte), kind en **+132 (0x84)** (byte).
    // Nosotros leíamos activo en +0x04 y kind como SHORT en +0x02 — pero +0x02
    // es `entity_type` (390 para jugadores, per CLAUDE.md), así que
    // `*(short*)(c+2) == 1` NUNCA era cierto → el pass 1 no matcheaba jamás y
    // no se creaba ninguna burbuja.
    // Entity stride 0x394 (=916, coincide con IDA). ID string en +0x1C1 (=449).

    DWORD base = DAT_07abf5d0;  // CharactersClient

    // Pass 1: search players (Kind == 1) — por nombre
    for (int i = 0; i < 400; i++) {
        DWORD c = base + i * 0x394;
        if (*(BYTE*)c != 0 && *(BYTE*)(c + 132) == 1) {
            if (strcmp((char*)(c + 0x1C1), ID) == 0) {
                FUN_00481ba0(ID, Text, c, Flag, -1);
                return;
            }
        }
    }

    // Pass 2: PRIMERA entidad con Kind == 2 (monstruo), SIN comparar nombre.
    // Fiel a IDA: hace `strcmp(...)` pero DESCARTA el resultado y llama igual
    // (rareza del original — el strcmp quedó sin usar).
    for (int i = 0; i < 400; i++) {
        DWORD c = base + i * 0x394;
        if (*(BYTE*)c != 0 && *(BYTE*)(c + 132) == 2) {
            FUN_00481ba0(ID, Text, c, Flag, -1);
            return;
        }
    }
}
