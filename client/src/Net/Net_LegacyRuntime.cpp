// Net_LegacyRuntime.cpp
//
// Final network/session functions extracted from stubs_game.cpp.  The code
// retains each IDA symbol/address in its leading comment; this relocation does
// not change packet, initialization, or anti-tamper behaviour.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" DWORD g_ItemAttribute_Backup;   // defined in Render_Frame.cpp
extern void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey);
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);
extern void __cdecl FUN_0046ca00(DWORD Object);
extern "C" void __cdecl FUN_004cdc70(float sx, float sy, float w, float h, int slotIdx);

// 2026-05-08 BUG-FIX MAYÚSCULO: en el binario original `OffsetInventoryItems`
// y `DAT_07ea8410` son el mismo símbolo (idem para Mix/Warehouse). En nuestra
// build son globals separados, así que comparaciones tipo
// `DAT_07ea9800 == &DAT_07ea8410` siempre fallaban → drop dispatcher tomaba
// el fallback "Other context" y enviaba srcType=3 con srcIdx==dstIdx, lo cual
// era no-op del lado del server y no liberaba el lock EnableUse.
// Forward-decls a nivel TU para usar las direcciones reales del pool.
extern "C" BYTE OffsetInventoryItems[];   // HUD_Pass3.cpp — main inv (8x8 + 12 wear)
extern "C" BYTE OffsetTradeItems[];       // HUD_Pass3.cpp
extern "C" BYTE OffsetMixItems[];         // HUD_Pass3.cpp
extern "C" BYTE OffsetWarehouseItems[];   // HUD_Pass3.cpp
extern "C" const char* Guild_GetMarkName(int row);
extern "C" BYTE Inventory[];              // HUD_Pass3.cpp
extern "C" int dword_7EAA0C8;             // HUD_Pass3.cpp
extern "C" int dword_7EAA0CC;             // HUD_Pass3.cpp

// IDA Hex-Rays intrinsic shims (mirror of stubs.cpp shims).
#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SHIBYTE(x)           (*(((char*)&(x))+1))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif
#ifndef COERCE_FLOAT
#define COERCE_FLOAT(x)      (*(float*)&(x))
#endif
#ifndef __PAIR64__
#define __PAIR64__(hi,lo)    (((unsigned __int64)(DWORD)(hi)<<32)|(DWORD)(lo))
#endif
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif


// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 16 — Inventory, equipment, item management, gold formatting
// ═══════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// InitGame @ 0x00424490 (829 bytes, ~227 lines)
// Resets all game state globals on entering a new game session.
// Clears: inventory, quest, attack state, NPC selection, repair mode, notices.
// Hash table operations: looks up CharacterMachine key, clones+XOR-encrypts
// the CHARACTER_ATTRIBUTE block (0x584 bytes), then re-inserts.
// Second hash pass decrements ref-count and frees if zero.
// Finally clears event chip, Muto numbers, whisper, match info, party list.
// ─────────────────────────────────────────────────────────────────────────────
void __stdcall InitGame(void)
{
    // --- Phase 1: Reset game state globals ---
    EnableUse = 0;
    DAT_07e11998 = -1;    // SendGetItem
    // IDA InitGame L31 es `SummonLife = 0`, y SummonLife vive en 0x05826D24
    // (verificado con ida_xrefs_to: lo escriben InitGame, ReceiveRevival x3 y
    // ProtocolCore, y lo lee RenderEquipedHelperLife).  El port escribia
    // DAT_07E11D28, que es **MouseUpdateTime** -- el contador del debounce de
    // movimiento de Player_InputTick.  Dos efectos: SummonLife nunca se
    // limpiaba al salir de la sesion, y ponerlo en 0 aca hacia que al volver
    // al mundo el primer click quedara bloqueado hasta contar de nuevo hasta
    // MouseUpdateTimeMax (que InitGame no toca y puede venir en ~28-49 del
    // ultimo camino recorrido) = hasta ~2 s sin poder caminar.
    DAT_05826d24 = 0;     // SummonLife
    DAT_05826c08 = 0;     // SoccerTime    (IDA InitGame @0x4244B4)
    DAT_05826d33 = 0;     // SoccerObserver (IDA InitGame @0x4244BA)
    DAT_07e11994 = -1;    // SelectedNpc
    DAT_07e11990 = -1;    // SelectedOperate
    DAT_07e1198c = -1;    // SelectedCharacter
    DAT_07e11988 = -1;    // SelectedItem
    DAT_00559c58 = -1;    // Attacking (IDA InitGame L38, global 0x00559C58).
                          // Antes escribia DAT_07e11984, que es el debounce de
                          // la flecha arriba del chat.
    DAT_07e11e18 = 1;     // m_bAutoAttack = true
    DAT_07e11d24 = 0;     // _CheckInventory
    // IDA InitGame L41 es `World = -1`, y World es 0x0055A7AC (DAT_0055a7ac).
    // El port escribia DAT_005615c4, que es g_lpszMp3[0] — el puntero al mp3 de
    // la taberna — asi que cada InitGame lo dejaba en -1 y PlayMp3 recibia (char*)-1.
    DAT_0055a7ac = -1;   // World
    // CSQuest__ClearQuest(g_csQuest);
    // IDA InitGame L43 es `LockInputStatus = 0`, y LockInputStatus vive en
    // 0x07E11D6F (xrefs: WndProc x4, InitGame, ReceiveJoinMapServer,
    // RenderIME_Status).  El port escribia DAT_07E11D1C, que es **LoadingWorld**
    // -- el contador que gatea el frame de render (`if (LoadingWorld > 30) return`).
    DAT_07e11d6f = 0;     // LockInputStatus
    DAT_07e11d18 = 0;     // RepairEnable variants
    DAT_07e11d14 = 0;
    DAT_00559c6d = 0xff;
    DAT_0055a3e4 = 0xffffffff;
    FUN_0051d740();       // ClearNotice

    // --- Phase 2: Hash table lookup for CharacterMachine ---
    // Looks up CharacterMachine key in MAIN_HASH_CLASS (anti-tamper obfuscation).
    // If found, clones the 0x584-byte block, XOR-encrypts it with key table
    // at DAT_00559050 (reverse iterate, XOR ^ subtract 0x23, add 0xB9),
    // then copies back. Reference count incremented.
    // This is anti-tamper hash table manipulation — not game logic.

    // --- Phase 3: Reset CharacterAttribute ---
    // CharacterAttribute->AbilityTime[0] = 0;
    // CharacterAttribute->AbilityTime[1] = 0;
    // CharacterAttribute->AddPoint = 0;

    // --- Phase 4: Second hash pass (decrement ref-count, free if zero) ---
    // Same hash table pattern as Phase 2, but decrements refcount.
    // If refcount hits 0, calls FUN_00404400 to free the entry.

    // --- Phase 5: Final cleanup ---
    DAT_07e11e1c = 0;     // _g_shEventChipCount
    DAT_07e11e20 = -1;    // g_shMutoNumber[0]
    DAT_07e11e22 = -1;    // g_shMutoNumber[1]
    DAT_07e11e24 = -1;    // g_shMutoNumber[2]
    DAT_07e11e26 = 0;     // m_bWhisperSound
    FUN_00482350();       // ClearWhisperID
    FUN_0047eb80();       // clearMatchInfo (0x0047EB80; antes se llamaba a
                          // FUN_004827a0, un stub vacio con la direccion mal)
    FUN_00433830();       // InitPartyList
}

// ─────────────────────────────────────────────────────────────────────────────
// ReceiveChat @ 0x00427630 (692 bytes, ~156 lines)
// Packet handler for opcode 0x00 (chat message from server).
// Packet layout: [C1][len][00][sender:10][msg:60][type:1][text...]
//
// If SceneFlag == 2 (in-game):
//   Sends a 4-byte ACK packet {0xC1, 0x04, 0x0E, ...} back to server
//   (keep-alive/chat ACK). Handles WSAEWOULDBLOCK by queuing to send buffer.
//
// Otherwise (login/charselect scene):
//   Extracts sender name (10 bytes @ offset 3) and message text (59 bytes @ offset 0xE).
//   Routes by chat type byte at offset 0x0D:
//     '~' (0x7E) → AddText(sender, msg, 4)        — whisper
//     '@' (0x40) → AddText(sender, msg, 5)        — GM/announce
//     '#' (0x23) → AssignChat(sender, msg, 1)     — party chat
//     default    → AssignChat(sender, msg, 0) + AddText(sender, msg, 3) — normal chat
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl ReceiveChat(BYTE *ReceiveBuffer)
{
    // BUG-FIX: DAT_07e11980 no existe en PE. SceneFlag real = DAT_005615c0.
    // El comentario "(in-game)" era incorrecto: 2 = Login en este cliente.
    if (DAT_005615c0 == 2) {  // g_GameState == 2 (Login scene)
        // Send 4-byte ACK: C1 04 0E xx
        char ackPkt[4];
        ackPkt[0] = (char)0xC1;
        ackPkt[1] = 4;
        ackPkt[2] = 0x0E;
        ackPkt[3] = 0;
        int sent = 0;
        int remain = 4;
        if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
            while (remain > 0) {
                int r = send(DAT_055ca168, ackPkt + sent, remain, 0);
                if (r == SOCKET_ERROR) {
                    if (WSAGetLastError() != WSAEWOULDBLOCK) {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));  // Net_Disconnect
                        return;
                    }
                    // Queue to send buffer
                    if (DAT_055cc16c + 4 > 0x2000) {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                        return;
                    }
                    memcpy((char*)DAT_055ca16c + DAT_055cc16c, ackPkt, 4);
                    DAT_055cc16c += 4;
                    return;
                }
                if (r == 0) return;
                if (DAT_055ce174 != 0) FUN_0043de60();  // Net_FlushSendQueue
                remain -= r;
                sent += r;
            }
        }
        return;
    }

    // --- Chat message processing (login/charselect scene) ---
    char sender[11];
    char msg[61];
    memset(sender, 0, sizeof(sender));
    memset(msg, 0, sizeof(msg));

    // Copy sender name (10 bytes at offset 3)
    memcpy(sender, ReceiveBuffer + 3, 10);

    // Copy message text (up to 59 bytes)
    memset(msg, 0, 61);

    // FIX 2026-07-19: las 3 ramas llamaban `FUN_00481a40(0, sender, 0)` — función y
    // argumentos equivocados, y el canal siempre 0. IDA ReceiveChat (0x427630) usa
    // UIChatLogWindow_AddText(strID, strText, <canal>) con el canal correcto por
    // prefijo. Sin esto el mensaje nunca entraba al chat log con su color/canal.
    extern void __cdecl UIChatLogWindow_AddText(const char* label, const char* msg, int mode);

    BYTE chatType = ReceiveBuffer[0x0D];

    if (chatType == '~') {
        // Party (canal 4)
        memcpy(msg, ReceiveBuffer + 0x0E, 59);
        UIChatLogWindow_AddText(sender, msg, 4);
    }
    else if (chatType == '@') {
        // Guild (canal 5)
        memcpy(msg, ReceiveBuffer + 0x0E, 59);
        UIChatLogWindow_AddText(sender, msg, 5);
    }
    else if (chatType == '#') {
        // Solo burbuja sobre el personaje (sin entrada en el log)
        memcpy(msg, ReceiveBuffer + 0x0E, 59);
        AssignChat_stub(sender, msg, 1);
    }
    else {
        // Chat normal: burbuja + log en canal 3 (color casi-blanco 0xFFEFDCCD;
        // renderLine lo saltea si byte_559BF1==0, que es el toggle de F2).
        memcpy(msg, ReceiveBuffer + 0x0D, 60);
        AssignChat_stub(sender, msg, 0);
        UIChatLogWindow_AddText(sender, msg, 3);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// FUN_00422c50 @ 0x00422C50 (395 bytes, ~99 lines)
// __fastcall(ecx=contextObj). Binary tree node cleanup for a connection/session
// object. Frees 4 heap allocations at offsets +0x3FC, +0x408, +0x40C, +0x410.
// Resets +0x400 to 1950000000 (likely a timeout sentinel), +0x404 to -1.
// Then rebuilds the BST rooted at +0x41C:
//   - Sets vtable at +0x414 to PTR_LAB_00552840
//   - Recursively inserts left/right children via FUN_004235d0
//   - Removes root node via FUN_004236c0 or FUN_004232f0
//   - Handles the "find rightmost in left subtree" replacement for BST delete
// Finally zeroes +0x41C (root) and +0x418 (count).
// ─────────────────────────────────────────────────────────────────────────────
void __fastcall FUN_00422c50(int param_1)
{
    // Free heap allocations
    if (*(void **)(param_1 + 0x3FC) != NULL) {
        operator_delete(*(void **)(param_1 + 0x3FC));
        *(DWORD *)(param_1 + 0x3FC) = 0;
    }
    if (*(void **)(param_1 + 0x408) != NULL) {
        operator_delete(*(void **)(param_1 + 0x408));
        *(DWORD *)(param_1 + 0x408) = 0;
    }
    if (*(void **)(param_1 + 0x40C) != NULL) {
        operator_delete(*(void **)(param_1 + 0x40C));
        *(DWORD *)(param_1 + 0x40C) = 0;
    }
    if (*(void **)(param_1 + 0x410) != NULL) {
        operator_delete(*(void **)(param_1 + 0x410));
        *(DWORD *)(param_1 + 0x410) = 0;
    }
    *(int *)(param_1 + 0x400) = 1950000000;  // timeout sentinel
    *(int *)(param_1 + 0x404) = -1;

    // BST rebuild at +0x414 / +0x41C
    // Walks tree, re-inserts children via FUN_004235d0, then removes root.
    // Handles standard BST node deletion with in-order predecessor swap.
    // Details: see Ghidra decompile @ 0x00422C50.

    *(DWORD *)(param_1 + 0x41C) = 0;  // root = NULL
    *(DWORD *)(param_1 + 0x418) = 0;  // count = 0
}

// ─────────────────────────────────────────────────────────────────────────────
// AppearMonster @ 0x0042A0C0 (234 bytes, ~44 lines)
// Sets initial animation + sound when a monster entity first appears on screen.
// Dispatched by MonsterIndex (entity type):
//   0x15 (21)       → action 1, sound 0x13  (bull fighter)
//   0x2C (44)       → action 1, sound 0x126 (agon)
//   0x35-0x36 (53-54) → action 1, Appear=60 frames (fade-in over ~2.4s)
//   0x55,0x5B,0x61,0x72,0x78,0x7E → sound 0x14B (boss monsters)
//   Object.Type==0x186 → action 0x81, alpha 1.0 (special NPC)
//   default         → alpha 0.0 (invisible, fade-in handled elsewhere)
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl AppearMonster(DWORD c)
{
    BYTE monIdx = *(BYTE *)(c + 0x02);  // MonsterIndex
    switch (monIdx) {
    case 0x15:
        *(int *)(c + 0x105) = 1;   // SetAction(obj, 1)
        *(int *)(c + 0x106) = 1;   // PriorAction = 1
        *(float *)(c + 0x04) = 1.0f; // Alpha = 1.0
        PlayBuffer(0x13, 0, 0);
        return;
    case 0x2C:
        *(int *)(c + 0x105) = 1;
        *(int *)(c + 0x106) = 1;
        *(float *)(c + 0x04) = 1.0f;
        PlayBuffer(0x126, 0, 0);
        return;
    case 0x35:
    case 0x36:
        *(int *)(c + 0x100) = 60;  // Appear = 60 frames
        *(int *)(c + 0x105) = 1;
        *(int *)(c + 0x106) = 1;
        *(float *)(c + 0x04) = 1.0f;
        return;
    case 0x55: case 0x5B: case 0x61:
    case 0x72: case 0x78: case 0x7E:
        PlayBuffer(0x14B, 0, 0);   // boss spawn sound
        return;
    }
    if (*(int *)(c + 0x00) == 0x186) {  // Object.Type == special NPC
        *(int *)(c + 0x105) = 0x81;
        *(int *)(c + 0x106) = 0x81;
        *(float *)(c + 0x04) = 1.0f;
        return;
    }
    *(float *)(c + 0x04) = 0.0f;  // default: invisible
}

// ─────────────────────────────────────────────────────────────────────────────
// FUN_00422074 @ 0x00422074 (34 bytes, ~13 lines)
// SEH cleanup handler: sets exception cookie to -1, calls DestroyWindow(),
// restores ExceptionList, returns saved value from stack frame.
// This is a compiler-generated __finally block, not game logic.
// ─────────────────────────────────────────────────────────────────────────────
void __stdcall FUN_00422074(void)
{
    // Compiler-generated SEH __finally handler
    // Sets EBP[-4] = 0xFFFFFFFF (exception state sentinel)
    // Calls DestroyWindow()
    // Restores ExceptionList from EBP[-0xC]
    // Returns EBP[-0x24]
    DestroyWindow(NULL);
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 18
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 19 — SendCheck (anti-tamper checksum packet) — 0x004220A0, 2988 bytes
// ═══════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// SendCheck @ 0x004220A0 (2988 bytes, 631 lines)
//
// Anti-tamper/checksum packet sent periodically to the game server.
// Builds a C1/C2 packet containing XOR-encrypted character stats,
// then sends it over the game socket with WSAEWOULDBLOCK queue handling.
//
// == Packet construction ==
//
// 1. Starts with header: [C1][len][F3][03]
// 2. Appends a NULL byte (encrypted with XOR key)
// 3. Appends GetTickCount() as 4-byte DWORD (XOR-encrypted)
// 4. Hash table lookup for CharacterMachine — anti-tamper obfuscation:
//    - If not found: allocate 0x585-byte entry, insert via FUN_00403f80
//    - If found: increment refcount, clone+encrypt if refcount < 2
// 5. Reads CharacterAttribute fields and appends (XOR-encrypted):
//    - If AbilityTime[0] bit 0 == 0 (normal):
//        AttackDamageMinRight (2 bytes)
//        MagicDamageMax       (2 bytes)
//    - If AbilityTime[0] bit 0 == 1 (buff active):
//        AttackDamageMinRight - 0x14 (2 bytes)
//        MagicDamageMax - 0x14       (2 bytes)
// 6. Second hash table pass: decrement refcount, free if zero
// 7. Sets packet length field (C1 = 1-byte len, C2 = 2-byte len)
// 8. Copies packet to abStack_40c buffer
// 9. Appends random byte (rand()) at end
// 10. Hash table lookup for g_byPacketSerialSend:
//     - Inserts serial number at offset [1] or [2] depending on header type
//     - Increments g_byPacketSerialSend
//     - Decrements refcount on old serial entry
// 11. Calls FUN_0053cc30 to encode/compress the payload
// 12. Sends via send() with full WSAEWOULDBLOCK queue handling
//     - C1 header: acStack_914 buffer (< 0x100 encoded size)
//     - C2 header: acStack_810 buffer (>= 0x100 encoded size)
// 13. Sets DAT_05826cfc = 1, DAT_05826d00 = tickCount on first call
//
// == XOR encryption pattern (repeated ~8 times in function) ==
//
// Key: 32-byte table (same as login XOR key):
//   {0xE7,0x6D,0x3A,0x89,0xBC,0xB2,0x9F,0x73,
//    0x23,0xA8,0xFE,0xB6,0x49,0x5D,0x39,0x5D,
//    0x8A,0xCB,0x63,0x8D,0xEA,0x7D,0x2B,0x5F,
//    0xC3,0xB1,0xE9,0x83,0x29,0x51,0xE8,0x56}
//
// For each byte at position i:
//   buf[i] ^= key[i % 32] ^ buf[i+1]  (chained XOR)
//
// The key is re-initialized (forward order, then reverse order) around
// each XOR loop — this is a compiler artifact / anti-tamper pattern,
// not meaningful crypto variation (see CLAUDE.md notes).
//
// == Hash table operations ==
//
// Uses MAIN_HASH_CLASS (anti-tamper obfuscation, not game logic):
//   FUN_004041e0 — hash lookup (returns slot index or 0xFFFFFFFF)
//   FUN_00403f80 — hash insert (allocates 0x585-byte entry)
//   FUN_00404280 — hash get value (returns entry pointer)
//   FUN_00404330 — hash clone+encrypt entry
//   FUN_00404400 — hash free entry (when refcount hits 0)
//   FUN_00423710 — hash remove entry
//
// == Network send ==
//
// Same pattern as all other packet sends:
//   send() in a loop, handle WSAEWOULDBLOCK by copying to
//   DAT_055ca16c queue (max 0x2001 bytes), or disconnect
//   via FUN_0043dc90 on hard error.
//   FUN_0043de60 called after successful partial send if
//   DAT_055ce174 != 0 (flush pending queue).
// ─────────────────────────────────────────────────────────────────────────────
void __stdcall SendCheck(void)
{
    if (DAT_07e11d10 == 0) return;  // g_bGameServerConnected

    DWORD tickCount = GetTickCount();

    // --- Build packet buffer ---
    // Header: C1 <len> F3 03
    BYTE pktBuf[0x400];
    int pktLen = 0;
    pktBuf[0] = 0xC1;
    pktBuf[2] = 0xF3;
    pktBuf[3] = 0x03;
    pktLen = 4;  // will be updated with encrypted fields

    // XOR encryption key (32 bytes, same as login packet key)
    static const BYTE xorKey[32] = {
        0xE7,0x6D,0x3A,0x89,0xBC,0xB2,0x9F,0x73,
        0x23,0xA8,0xFE,0xB6,0x49,0x5D,0x39,0x5D,
        0x8A,0xCB,0x63,0x8D,0xEA,0x7D,0x2B,0x5F,
        0xC3,0xB1,0xE9,0x83,0x29,0x51,0xE8,0x56
    };

    // Append NULL byte (XOR-encrypted)
    if (pktLen + 1 < 0x401) {
        pktBuf[pktLen] = 0;
        for (int i = pktLen; i < pktLen + 1; i++)
            pktBuf[i] ^= xorKey[i & 0x1F] ^ pktBuf[i + 1];
        pktLen += 1;
    }

    // Append GetTickCount (4 bytes, XOR-encrypted)
    if (pktLen + 4 < 0x401) {
        *(DWORD *)(pktBuf + pktLen) = tickCount;
        for (int i = pktLen; i < pktLen + 4; i++)
            pktBuf[i] ^= xorKey[i & 0x1F] ^ pktBuf[i + 1];
        pktLen += 4;
    }

    // --- Hash table anti-tamper: lookup CharacterMachine key ---
    // (obfuscation pattern — see CLAUDE.md anti-tamper notes)
    // Inserts/clones CharacterAttribute data into hash table,
    // XOR-encrypts the 0x584-byte block with secondary key.

    // --- Append character stats (XOR-encrypted) ---
    // Reads from CharacterAttribute struct:
    //   If AbilityTime[0] bit0 == 0 (normal mode):
    //     +AttackDamageMinRight (2 bytes)
    //     +MagicDamageMax       (2 bytes)
    //   If AbilityTime[0] bit0 == 1 (buff active):
    //     +(AttackDamageMinRight - 0x14) (2 bytes)
    //     +(MagicDamageMax - 0x14)       (2 bytes)
    // Each 2-byte field XOR-encrypted with same key+chaining pattern.

    // --- Hash table: decrement refcount, free if zero ---

    // --- Set packet length field ---
    pktBuf[1] = (BYTE)pktLen;  // C1 format (1-byte length)

    // --- Copy to send buffer, append random byte ---
    BYTE sendBuf[0x400];
    memcpy(sendBuf, pktBuf, pktLen);
    sendBuf[pktLen] = (BYTE)(rand() & 0xFF);

    // --- Insert packet serial number ---
    // Hash lookup g_byPacketSerialSend, insert at offset [1] or [2]
    int serialOffset = (sendBuf[0] != 0xC1) ? 2 : 1;
    sendBuf[serialOffset + 1] = DAT_07db8600;  // g_byPacketSerialSend
    DAT_07db8600++;

    // --- Encode payload via FUN_0053cc30 ---
    int encodedLen = FUN_0053cc30(0, sendBuf + serialOffset + 1, pktLen - (serialOffset + 1));

    // --- Send over socket ---
    char outBuf[0x404];
    if (encodedLen < 0x100) {
        // C3 header (small packet)
        outBuf[0] = (char)0xC3;
        outBuf[1] = (char)(encodedLen + 2);
        FUN_0053cc30((int)(outBuf + 2), sendBuf + serialOffset + 1, pktLen - (serialOffset + 1));
        int totalLen = encodedLen + 2;
        int sent = 0;
        if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
            while (sent < totalLen) {
                int r = send(DAT_055ca168, outBuf + sent, totalLen - sent, 0);
                if (r == SOCKET_ERROR) {
                    if (WSAGetLastError() == WSAEWOULDBLOCK) {
                        if ((int)(totalLen + DAT_055cc16c) < 0x2001) {
                            memcpy((char *)DAT_055ca16c + DAT_055cc16c, outBuf, totalLen);
                            DAT_055cc16c += totalLen;
                        } else {
                            FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                        }
                    } else {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    }
                    break;
                }
                if (r == 0) break;
                if (DAT_055ce174 != 0) FUN_0043de60();
                totalLen -= r;
                sent += r;
            }
        }
    } else {
        // C4 header (large packet, 2-byte length)
        int totalLen = encodedLen + 3;
        outBuf[0] = (char)0xC4;
        outBuf[1] = (char)((totalLen >> 8) & 0xFF);
        outBuf[2] = (char)(totalLen & 0xFF);
        FUN_0053cc30((int)(outBuf + 3), sendBuf + serialOffset + 1, pktLen - (serialOffset + 1));
        int sent = 0;
        if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
            while (sent < totalLen) {
                int r = send(DAT_055ca168, outBuf + sent, totalLen - sent, 0);
                if (r == SOCKET_ERROR) {
                    if (WSAGetLastError() == WSAEWOULDBLOCK) {
                        if ((int)(totalLen + DAT_055cc16c) < 0x2001) {
                            memcpy((char *)DAT_055ca16c + DAT_055cc16c, outBuf, totalLen);
                            DAT_055cc16c += totalLen;
                        } else {
                            FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                        }
                    } else {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    }
                    break;
                }
                if (r == 0) break;
                if (DAT_055ce174 != 0) FUN_0043de60();
                totalLen -= r;
                sent += r;
            }
        }
    }

    // First-call flag
    if (DAT_05826cfc == 0) {
        DAT_05826cfc = 1;
        DAT_05826d00 = tickCount;
    }
}

// Catch@0042206e @ 0x0042206E (6 bytes) — SEH catch block, returns FUN_00422074
void * __stdcall Catch_0042206e(void) {
    return (void *)FUN_00422074;
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 19
// ═══════════════════════════════════════════════════════════════════════════════
