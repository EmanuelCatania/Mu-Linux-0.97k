// Party.cpp
// Sistema de Party/grupo — handlers de paquetes y datos de barras de HP
//
// Handlers de paquetes (server→client):
//   0x44  Party_HPBars        — inline in Net_Process; party member HP bar update
//   0x46  Terrain_TileUpdate  @ 0x00436d60 — terrain tile rect/point changes
//   0x71  Party_Keepalive     @ 0x00433900 — passive legacy reply [C1][03][71]
//   0x73  ReceiveGGAuth       @ 0x00433a80 — pertenece a protocolo, no a Party
//
// ── PARTY HP BAR DATA (opcode 0x44) ──────────────────────────────────────────
//
//   Table: DAT_07e11e98  stride 0x24 per member slot
//     slot = upper nibble of each packet byte (max 10 members?)
//     HP%  = lower nibble, clamped to 10 (represents HP in 10% steps)
//
//   Packet: [C1][len][44][count][byte_N...]
//     count = byte[3]
//     per byte: upper_nibble = member_slot, lower_nibble = hp_percent_step
//     DAT_07e11e98[slot * 0x24] = min(hp_step, 10)
//
// ── TERRAIN TILE UPDATE (opcode 0x46, FUN_00436d60) ──────────────────────────
//
//   byte[3] == 0x00 → Rectangle tile update:
//     if g_GameSubState in [10..16] and byte[4]==8:
//       FUN_004fa5c0(g_GameSubState, 0x24, 0, 1)  — map zone transition
//     Loop byte[6] count, stride 4 from byte[8]:
//       byte[-1] = x1, byte[0] = y1, byte[1] = x2, byte[2] = y2
//       Terrain_UpdateTileAttributeRect (IDA: FUN_004f6f30)
//       (x1, y1, x2-x1+1, y2-y1+1, byte[4], !(byte[5]))
//         — update terrain attrib in rect (floor_id=byte[4], door_open=!byte[5])
//
//   byte[3] == 0x01 → Single tile update:
//     Loop byte[6] count, stride 2 from byte[7]:
//       byte[0] = tile_x, byte[1] = tile_y
//       if byte[5]==0: Terrain_SetTileAttributeBits (IDA: FUN_004f6ef0)
//       else:          Terrain_ClearTileAttributeBits (IDA: FUN_004f6f10)
//
//   Terrain tile-attribute writers (IDA: FUN_004f6ef0 / FUN_004f6f10 / FUN_004f6f30).
//
// ── PARTY KEEPALIVE (opcode 0x71, FUN_00433900) ──────────────────────────────
//
//   Legacy response path: if an external server sends opcode 0x71, the client
//   replies with [0xC1][0x03][0x71].  Current MuEmu has no 0x71 route, so this
//   handler remains passive and never schedules or emits traffic on its own.
//
// ── PARTY CHAR SYNC (opcode 0x73, FUN_00433a80, 380 lines) ───────────────────
//
//   Dual-purpose handler dispatched on packet length (param_2):
//
//   param_2 == 0  →  Char-data request ACK:
//     Builds a small C1 packet with opcode 0xF1 sub-opcode 0x01:
//       [0xC1][len][0xF1][0x01][0x00][rand_byte]
//     XOR-encrypts with standard 32-byte key {0xe7,0x6d,0x3a,...,0xe8,0x56}
//     then RC4-encodes via FUN_0053cc30 (same cipher as login)
//     and wraps in C3 or C4 envelope depending on final length.
//     Sends with standard WSAEWOULDBLOCK retry loop.
//     Purpose: re-send character authentication sync (used when joining party).
//
//   param_2 != 0  →  BGM notification:
//     FUN_0053d5c0(*(CHAR**)(param_1 + 4))
//     Passes string pointer from packet to background music player.
//     Purpose: server instructs client to play a specific BGM track.
//
//   HashTable operations (DAT_055c9bc8, key DAT_05826ceb) interspersed
//   throughout — anti-tamper obfuscation, not game logic.
//
// ── GUILD (opcodes 0x90-0x99) ────────────────────────────────────────────────
//
//   Packet handlers dispatched by Net_Process for guild operations.
//   All take a raw packet pointer (param_1).
//
// ── Guild create result (opcode 0x90, FUN_00436820) ──────────────────────────
//   Sends a 3-byte ACK [C1][03][31] to the server.
//   Switches on packet[3] (sub-type 1–5) to select a pre-loaded string buffer,
//   then calls FUN_0051d6f0 to decode and display the guild notification in chat.
//     1 → DAT_07d5b680  (create success variant A)
//     2 → DAT_07d5b7ac  (create success variant B)
//     3 → DAT_07d5c10c  (create success variant C)
//     4 → DAT_07d5c238  (create success variant D)
//     5 → DAT_07d5b8d8  (create success variant E)
//
// ── Guild member add result (opcode 0x91, FUN_00436cb0) ──────────────────────
//   Sub-type dispatch on packet[3]:
//     ==1: Join request result — if packet[4]==0 show DAT_07d58ea8 (OK message),
//          else format param_2_07d58fd4 with packet[4] (error/rank code) via wsprintfA.
//     ==2: Join confirm result — if packet[4]==0 show DAT_07d6813c (OK message),
//          else format param_2_07d68268 with packet[4] via wsprintfA.
//   Result message shown via FUN_0051d6f0.
//
// ── Guild member list (opcode 0x93, FUN_00436a80) ────────────────────────────
//   Dispatch on packet[4]:
//     ==0xFF: FUN_0051da80(packet[3], packet+5) — add one member record (0x18 bytes).
//     else:   FUN_0051d9e0(packet[4], packet[3], packet+5) — full list update
//             (packet[4] = member count, packet[3] = param, packet+5 = data array).
//   Guild member list table at DAT_083a7af8, stride 0x18 per member.
//   UI sub-state: 0x8c (member list panel) or 0x9a (single add).
//
// ── Guild char-select result (opcode 0x94, FUN_004372c0) ─────────────────────
//   Sets DAT_07eaa128 = packet[3]+1 (guild load stage) and
//        DAT_07eaa12c = packet[4..5] (short: guild ID or member count).
//   When stage reaches 3 (packet[3]==2):
//     Zeroes 64 bytes at DAT_07ea97c0 (guild entity pool).
//     Calls Input_ClearState(0) to reset char-select.
//     Sets _DAT_00559c94=0xC (login sub-state → CharSelectInit),
//     clears DAT_00559c84/0x88, DAT_07e11d72/74, DAT_07eaa108.
//     Sets DAT_07e11d73=1 (char-select flag D).
//   When stage==1 and packet[6..10] are all != -1:
//     _DAT_00559f58 = packet[6..9] (dword, guild target tile X)
//     DAT_00559f5c  = packet[10..11] (word, guild target tile Y)
//   Clears DAT_07eaa117 and DAT_07eaa116.
//
// ── Guild update pos (opcode 0x95, FUN_00437380) ──────────────────────────────
//   If packet[4..5] (ushort) != 0xFFFF:
//     DAT_07eaa12c = packet[4..5]  — update guild ID / member count
//
// ── Guild set target pos (opcode 0x96, FUN_004373a0) ─────────────────────────
//   If packet[4..5], packet[6..7], packet[8..9] are all != -1:
//     _DAT_00559f58 = packet[4..7] (dword, guild target X)
//     DAT_00559f5c  = packet[8..9] (word, guild target Y)
//
// ── Guild join toggle (opcode 0x99, FUN_004373d0) ────────────────────────────
//   packet[3] == 0 → FUN_005142d0(0x90)  — guild join accept UI
//   packet[3] == 1 → FUN_005142d0(0x91)  — guild join decline UI

#include "stdafx.h"
#include "Party.h"
#include "Net/Net.h"    // Net_Disconnect, DAT_055ca168 / WSAEWOULDBLOCK queue

// g_GameSubState is declared as 'int DAT_0055a7ac' in globals.h; use the macro alias.
#define g_GameSubState DAT_0055a7ac
extern BYTE* g_PartyHPTable;           // DAT_07e11e98  stride 0x24

#define PARTY_HP_STRIDE  0x24

// sub_4AFDC0/sub_4E5980 re-arm these runtime fields with -2 whenever the
// viewport may have changed. sub_4AFB00 resolves them strictly by name against
// currently visible players; these fields never represent Party membership.
void Party_RefreshViewportLinks(void)
{
    for (int i = 0; i < PartyNumber; ++i) {
        BYTE* const slot = Party + i * 36;
        *(int*)(slot + 28) = -2;
        *(int*)(slot + 32) = 0;
    }
    Party_MatchEntityNames();
}

// ============================================================
// ReceivePartyList97k  (ReceivePartyList @ 00434660, opcode 0x42)
//
// MuEmu PMSG_PARTY_LIST_SEND:
//   [C1][size][42][result][count][members...]
// Native-aligned PMSG_PARTY_LIST members have a 24-byte stride:
//   name[10], number, map, x, y, pad[2], CurLife(DWORD), MaxLife(DWORD).
//
// The client party table is the original 36-byte slot layout.  Its first
// 24 bytes are populated exactly as 00434660; the remaining UI-only fields
// are maintained by the existing HUD/HP-bar paths.
// ============================================================
void ReceivePartyList97k(BYTE* pkt, int size)
{
    if (!pkt || size < 5) return;

    const int count = pkt[4];
    constexpr int kMaxPartyMembers = 5; // MuEmu MAX_PARTY_USER
    constexpr int kHeaderSize = 5;
    constexpr int kWireStride = 24;
    if (count < 0 || count > kMaxPartyMembers ||
        kHeaderSize + count * kWireStride > size) {
        return;
    }

    // 00434660 writes PartyNumber before its per-member copy loop.  The
    // trailing two dwords are runtime link fields used by sub_4AFB00; the
    // original table starts them at -2 (unmatched), rather than zero.
    PartyNumber = count;
    for (int i = 0; i < count; ++i) {
        const BYTE* const wire = pkt + kHeaderSize + i * kWireStride;
        BYTE* const local = Party + i * 36;

        memcpy(local, wire, 10);              // name
        local[10] = 0;                        // original forced terminator
        local[11] = wire[10];                 // member number
        local[12] = wire[11];                 // map
        local[13] = wire[12];                 // X
        local[14] = wire[13];                 // Y
        memcpy(local + 16, wire + 16, 4);     // CurLife
        memcpy(local + 20, wire + 20, 4);     // MaxLife
        *(int*)(local + 28) = -2;             // CharactersClient index sentinel
        *(int*)(local + 32) = 0;              // cached entity status
    }

    Party_MatchEntityNames();
}


// ============================================================
// PacketHandler_0x44  (inline in Net_Process)
// Party member HP bar update.
// Packet: [C1][len][44][count][byte...]
//   Each byte: high nibble = member slot (0-9), low nibble = HP step (0-10)
//   Stored: Party[slot * 36 + 24] = min(hp_step, 10)
// ============================================================
void PacketHandler_0x44(BYTE* pkt, int size)
{
    if (!pkt || size < 4) return;
    int count = (BYTE)pkt[3];
    if (count < 0 || 4 + count > size) return;
    for (int i = 0; i < count; i++)
    {
        // ProtocolCore @ 004389A0: entries begin at ReceiveBuffer+4.
        BYTE b = pkt[4 + i];
        int  slot    = (b >> 4) & 0xF;
        int  hp_step = (b & 0xF);
        if (hp_step > 10) hp_step = 10;
        if (slot < 5) {
            Party[slot * 36 + 24] = (BYTE)hp_step;
        }
    }
}


// ============================================================
// Terrain_TileUpdate  @ 0x00436d60  (opcode 0x46)
// Updates terrain tile attributes in response to server events
// (doors opening/closing, map transitions, etc.)
// See Terrain.cpp for the underlying tile write functions.
// ============================================================
void Terrain_TileUpdate(BYTE* pkt)
{
    if (pkt[3] == 0x00)
    {
        // Rectangle update
        if (g_GameSubState > 10 && g_GameSubState < 0x11 && pkt[4] == 8)
            FUN_004fa5c0(g_GameSubState, 0x24, 0, 1);  // map zone transition

        int count = (BYTE)pkt[6];
        BYTE* entry = pkt + 8;
        for (int i = 0; i < count; i++, entry += 4)
        {
            int x1 = entry[-1], y1 = entry[0];
            int x2 = entry[1],  y2 = entry[2];
            int w  = (x2 - x1) + 1;
            int h  = (y2 - y1) + 1;
            Terrain_UpdateTileAttributeRect(x1, y1, w, h, pkt[4], !pkt[5]);
        }
    }
    else if (pkt[3] == 0x01)
    {
        // Single tile update
        int count = (BYTE)pkt[6];
        BYTE* entry = pkt + 7;
        for (int i = 0; i < count; i++, entry += 2)
        {
            int tile_x = entry[0], tile_y = entry[1];
            if (pkt[5] == 0)
                Terrain_SetTileAttributeBits(tile_x, tile_y, pkt[4]);  // set
            else
                Terrain_ClearTileAttributeBits(tile_x, tile_y, pkt[4]);  // clear
        }
    }
}


// ============================================================
// Party_Keepalive  @ 0x00433900  (opcode 0x71)
// Passive legacy reply. MuEmu does not currently emit or receive this route.
// Standard send() + WSAEWOULDBLOCK queue at DAT_055ca16c.
// ============================================================
void Party_Keepalive(void)
{
    const BYTE pkt[3] = { 0xC1, 0x03, 0x71 };
    // IDA: FUN_00433900 envía la trama C1 por la ruta normal de socket/cola.
    // Este opcode no va cifrado según la política de tramas del cliente.
    Net_SendC1Packet(pkt, sizeof(pkt));
}


// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  CÓDIGO MUERTO — desde 2026-08-26 no tienen callers                      ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// Las siete funciones `Guild_*` que siguen hasta el final del archivo estaban
// enganchadas a los opcodes 0x90-0x99. Ese rango NO es guild: IDA y MuEmu
// coinciden en que son eventos (Devil Square, Blood Castle, Golden Archer).
// La tabla opcode -> función real está en la cabecera de
// `src/Net/Net_Events.cpp`, que es quien los atiende ahora.
//
// El guild de verdad usa 0x50-0x56 y ya estaba bien atendido en Net_Process.cpp
// (`ReceiveGuildResult`, `ReceiveGuildList`, `ReceiveCreateGuildResult`, ...);
// esta tanda no lo tocó.
//
// No se borran todavía porque el cuerpo puede servir de referencia si alguna
// vez se porta el protocolo de guild de otra versión. Ojo con reengancharlas:
// `Guild_CreateOk` ENVÍA un `[C1][03][31]`, así que colgada del opcode
// equivocado no sólo muestra un cartel de más, también le manda basura al
// server.
//
// ============================================================
// Guild_CreateOk  @ 0x00436820  (opcode 0x90)
// Server ACKs guild creation; displays result message.
//
// Sends back a 3-byte packet [C1][03][31] (guild creation ACK).
// Then switches on pkt[3] (sub-type 1-5) to pick a pre-loaded
// string buffer and calls FUN_0051d6f0 to display it in chat.
// Standard WSAEWOULDBLOCK retry loop for the send().
// ============================================================
void Guild_CreateOk(BYTE* pkt)
{
    // Send guild ACK [C1][03][31]
    BYTE ack[3] = { 0xC1, 0x03, 0x31 };
    int sent = 0;
    UINT remaining = 3;
    DAT_07eaa117 = 0;
    if (DAT_055ca168 != (SOCKET)(~0))
    {
        do {
            int r = send(DAT_055ca168, (const char*)ack + sent, (int)remaining, 0);
            if (r == -1)
            {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK)
                {
                    if (DAT_055cc16c + 3 < 0x2001)
                    {
                        memcpy(DAT_055ca16c + DAT_055cc16c, ack, 3);
                        DAT_055cc16c += 3;
                    }
                    else
                        Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                }
                else
                    Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                break;
            }
            if (r == 0) break;
            if (DAT_055ce174 != 0) FUN_0043de60();
            remaining -= r;
            sent += r;
        } while ((int)remaining > 0);
    }

    // Select message buffer by sub-type
    char* msg = NULL;
    switch (pkt[3])
    {
    case 1: msg = (char*)&DAT_07d5b680; break;
    case 2: msg = (char*)&DAT_07d5b7ac; break;
    case 3: msg = (char*)&DAT_07d5c10c; break;
    case 4: msg = (char*)&DAT_07d5c238; break;
    case 5: msg = (char*)&DAT_07d5b8d8; break;
    default: return;
    }
    FUN_0051d6f0(msg);
}


// ============================================================
// Guild_AddMemberResult  @ 0x00436cb0  (opcode 0x91)
// Server reports result of a guild join request or confirmation.
//
// pkt[3]==1: join-request result
//   pkt[4]==0 → show DAT_07d58ea8 (success)
//   pkt[4]!=0 → wsprintfA format param_2_07d58fd4 with pkt[4] (error code)
// pkt[3]==2: join-confirm result
//   pkt[4]==0 → show DAT_07d6813c (success)
//   pkt[4]!=0 → wsprintfA format param_2_07d68268 with pkt[4] (error code)
// Result shown via FUN_0051d6f0.
// ============================================================
void Guild_AddMemberResult(BYTE* pkt)
{
    char buf[256];

    if (pkt[3] == 1)
    {
        if (pkt[4] == 0)
        {
            FUN_0051d6f0((char*)&DAT_07d58ea8);
        }
        else
        {
            wsprintfA(buf, &param_2_07d58fd4, (UINT)pkt[4]);
            FUN_0051d6f0((char*)buf);
        }
    }
    else if (pkt[3] == 2)
    {
        if (pkt[4] == 0)
        {
            FUN_0051d6f0((char*)&DAT_07d6813c);
        }
        else
        {
            wsprintfA(buf, &param_2_07d68268, (UINT)pkt[4]);
            FUN_0051d6f0((char*)buf);
        }
    }
}


// ============================================================
// Guild_MemberList  @ 0x00436a80  (opcode 0x93)
// Server sends guild member list (full update or single add).
//
// pkt[4]==0xFF → FUN_0051da80(pkt[3], pkt+5)
//   Add one member record (0x18 bytes at pkt+5).
// pkt[4]!=0xFF → FUN_0051d9e0(pkt[4], pkt[3], pkt+5)
//   Full member list: pkt[4]=count, pkt[3]=param, pkt+5=data array.
// Guild member list at DAT_083a7af8, stride 0x18 per entry.
// UI sub-state: 0x9a (add) or 0x8c (full list).
// ============================================================
void Guild_MemberList(BYTE* pkt)
{
    if (pkt[4] == 0xFF)
        FUN_0051da80((UINT)pkt[3], pkt + 5);
    else
        FUN_0051d9e0((int)pkt[4], (int)pkt[3], pkt + 5);
}


// ============================================================
// Guild_CharSelectResult  @ 0x004372c0  (opcode 0x94)
// Server sends guild-load pipeline stage result.
//
// DAT_07eaa128 = pkt[3]+1  (stage counter)
// DAT_07eaa12c = pkt[4..5] (short: guild ID or member count)
//
// Stage==3 (pkt[3]==2): reset char-select
//   Zero DAT_07ea97c0[0..63], call Input_ClearState(0)
//   Set _DAT_00559c94=0xC (CharSelectInit), clear various flags
//   Set DAT_07e11d73=1
//
// Stage==1 and pkt[6..10] all != -1: store target tile coords
//   _DAT_00559f58 = pkt[6..9] (dword)
//   DAT_00559f5c  = pkt[10..11] (word)
//
// Always clears DAT_07eaa117 and DAT_07eaa116.
// ============================================================
void Guild_CharSelectResult(BYTE* pkt)
{
    FUN_004cba60();   // char-select reset helper
    DAT_07eaa12c = (int)*(short*)(pkt + 4);
    DAT_07eaa128 = (int)(BYTE)pkt[3] + 1;

    if (DAT_07eaa128 == 3)
    {
        // Zero guild entity pool
        memset(DAT_07ea97c0, 0, sizeof(DAT_07ea97c0));
        Input_ClearState(0);                // reset char select
        _DAT_00559c94 = 0xC;           // login sub-state → CharSelectInit
        DAT_00559c84  = 0;
        DAT_00559c88  = 1;
        DAT_07e11d72  = 0;
        DAT_07e11d74  = 0;
        DAT_07eaa108  = 0;
        DAT_07e11d73  = 1;
    }

    DAT_07eaa117 = 0;
    DAT_07eaa116 = 0;

    if (DAT_07eaa128 == 1
        && *(short*)(pkt + 6)  != -1
        && *(short*)(pkt + 8)  != -1
        && *(short*)(pkt + 10) != -1)
    {
        _DAT_00559f58 = *(DWORD*)(pkt + 6);
        DAT_00559f5c  = *(WORD*) (pkt + 10);
    }
}


// ============================================================
// Guild_UpdatePos  @ 0x00437380  (opcode 0x95)
// Updates guild member count / guild ID if packet value is valid.
//
// pkt[4..5] (ushort) != 0xFFFF → DAT_07eaa12c = pkt[4..5]
// ============================================================
void Guild_UpdatePos(BYTE* pkt)
{
    USHORT val = *(USHORT*)(pkt + 4);
    if (val != 0xFFFF)
        DAT_07eaa12c = (int)val;
}


// ============================================================
// Guild_SetTargetPos  @ 0x004373a0  (opcode 0x96)
// Sets guild map target position if all three shorts are valid.
//
// Validates pkt[4..5], pkt[6..7], pkt[8..9] are all != -1, then:
//   _DAT_00559f58 = pkt[4..7] (dword, target X)
//   DAT_00559f5c  = pkt[8..9] (word,  target Y)
// ============================================================
void Guild_SetTargetPos(BYTE* pkt)
{
    if (*(short*)(pkt + 4) != -1
        && *(short*)(pkt + 6) != -1
        && *(short*)(pkt + 8) != -1)
    {
        _DAT_00559f58 = *(DWORD*)(pkt + 4);
        DAT_00559f5c  = *(WORD*) (pkt + 8);
    }
}


// ============================================================
// Guild_JoinToggle  @ 0x004373d0  (opcode 0x99)
// Server notifies client of guild join accept/decline.
//
// pkt[3]==0 → FUN_005142d0(0x90) — show guild join accept UI
// pkt[3]==1 → FUN_005142d0(0x91) — show guild join decline UI
// ============================================================
void Guild_JoinToggle(BYTE* pkt)
{
    if (pkt[3] == 0)
        FUN_005142d0(0x90);
    else if (pkt[3] == 1)
        FUN_005142d0(0x91);
}
