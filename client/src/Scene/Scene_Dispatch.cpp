// Scene_Dispatch.cpp
// Scene_Dispatch @ 0x005274A8
//
// Main per-frame dispatcher. Called from the PeekMessage loop in WinMain.
// Dispatches to the active scene via g_GameState, then handles keepalive
// re-authentication and anti-tamper obfuscation (HashTable ref-count).
//
// Signature: void __cdecl Scene_Dispatch(HDC param_1)
//
// Globals:
//   DAT_005615c0  — g_GameState (dispatch switch)
//   DAT_05826e08  — frame time accumulator
//   DAT_083a45d4  — login background animation angle
//   DAT_055ca028  — keepalive counter (> 0x1f → send re-auth)
//   DAT_055ca168  — socket handle
//   DAT_055ca16c  — send buffer (WSAEWOULDBLOCK queue, max 0x2001)
//   DAT_055cc16c  — send buffer byte count
//   DAT_055ca038  — post-send flag (cleared at end of function)
//   DAT_05826ceb  — packet sequence counter
//   DAT_00559050  — 16-byte XOR key for keepalive re-randomization

#include "stdafx.h"
#include "Scene/Scene_Dispatch.h"
#include "Scene/Scene_Intro.h"
#include "Scene/Scene_Loading.h"
#include "Game/Game_MainLoop.h"
#include "Net/Net.h"
#include <stdlib.h>   // for rand()
// (_rand ya está definido como `_rand() rand()` en stdafx.h)
// OpenGL_Release: declared static in WinMain.cpp; forward-declare here
extern void OpenGL_Release(void);

// Same 32-byte XOR key used in login packets
static const BYTE s_PktKey[32] = {
    0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
    0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
    0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
    0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
};

void __cdecl Scene_Dispatch(HDC param_1)
{
    // ── FRAME ANGLE ANIMATION ─────────────────────────────────────────────────
    // Original stored DAT_05826e08 * _DAT_005524bc into local_41c (unused after);
    // omitted here — same effect, no observable side effect.
    {
        float10 t = (float10)FUN_005433b0();
        DAT_083a45d4 = (float)(t * (float10)_DAT_00552934 + (float10)_DAT_00552534);
    }

    // ── SCENE DISPATCH ────────────────────────────────────────────────────────
    switch (DAT_005615c0) {
    case 1: Scene_Intro(param_1);    break;
    case 2:
    case 4:
    case 5: Game_MainLoop(param_1);  break;
    case 3: Scene_Loading(param_1);  break;
    }

    // ── KEEPALIVE COUNTER: HASH TABLE REF-COUNT TRACKING ─────────────────────
    // (anti-tamper obfuscation — tracks DAT_055ca028 via a reference-count table)
    {
        unsigned idx = HashTable_GetIndex(&DAT_055c9bc8, &DAT_055ca028);
        if (idx == 0xffffffff) {
            void* node = operator_new(5);
            *((BYTE*)node + 4) = 1;
            FUN_00403f80(&DAT_055c9bc8, node, &DAT_055ca028);
        } else {
            BYTE* node = *(BYTE**)(DAT_055c9bcc + idx * 4);
            node[4]++;
            if (node[4] < 2)
                FUN_00409e20(&DAT_055ca028, node);
        }
    }

    // ── RE-AUTH PACKET SEND (when keepalive counter > 0x1f) ──────────────────
    //
    // DISABLED for MuEmu compat (2026-04-25):
    //   Original 0.97 client sends a raw [C3][len][crc payload] re-auth packet
    //   every frame after Net_Connect via raw send() (no MuEmu cipher wrap).
    //   The packet body is XOR-encrypted with the login key, then CRC-wrapped.
    //
    //   Symptom: server FD_CLOSEs ~656 ms after F3/00 charlist arrives.  Login
    //   itself works because we wrap the F1/01 send through MuEmu::EncryptSend
    //   explicitly — but THIS path bypasses it, so once DAT_055ca028 (re-auth
    //   keepalive counter) randomizes >0x1f, every frame leaks an unencrypted
    //   C3 onto the wire that MuEmu can't decode → server kills the socket.
    //
    //   Original target server expected this re-auth (it's a GameGuard ping).
    //   MuEmu doesn't, so we just skip the entire block.  The hash-table
    //   bookkeeping below still runs (anti-tamper ref-count, harmless).
    #if 0
    if (DAT_055ca028 > 0x1f) {
        // Build a 6-byte re-auth packet: [0xC1, len, 0xF1, b3, b2, b0]
        // Each byte after the header is XOR-encrypted with s_PktKey.
        BYTE pkt[8];
        pkt[0] = 0xC1;
        pkt[1] = 6;      // packet length
        pkt[2] = 0xF1;   // opcode
        pkt[3] = 3;      // field 3
        pkt[4] = 2;      // field 4
        pkt[5] = 0;      // field 5

        // XOR-encrypt fields 3..5
        for (int i = 3; i < 6; i++) {
            int ki = i & 0x1f;
            pkt[i] ^= s_PktKey[ki] ^ pkt[i - 1];
        }

        // Append random byte
        pkt[6] = (BYTE)_rand();

        // Sequence counter tracking (anti-tamper)
        {
            unsigned idx2 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_05826ceb);
            if (idx2 == 0xffffffff) {
                void* node = operator_new(2);
                *((BYTE*)node + 1) = 1;
                FUN_00403f80(&DAT_055c9bc8, node, &DAT_05826ceb);
            } else {
                BYTE* node = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_05826ceb);
                node[1]++;
                if (node[1] < 2)
                    FUN_00404330((BYTE*)&DAT_05826ceb, node);
            }
        }

        // Place seq byte, advance counter
        int seqOff = 1; // ((pkt[0] != 0xC1) ? 1 : 0) + 2 - 1 = 1
        pkt[seqOff] = DAT_05826ceb;
        DAT_05826ceb++;

        // Decrement next seq slot ref-count
        {
            unsigned idx3 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_05826ceb);
            if (idx3 != 0xffffffff) {
                BYTE* node = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_05826ceb);
                node[1]--;
                if (node[1] == 0)
                    FUN_00423710(node, &DAT_05826ceb);
            }
        }

        // Compute CRC and send
        int payStart = seqOff;
        int payLen   = 6 - seqOff;
        int crc = FUN_0053cc30(0, pkt + payStart, payLen);

        if (crc < 0x100) {
            // Small packet: [0xC3][total_len][payload...]
            char sbuf[260];
            int  total = crc + 2;
            sbuf[0] = (char)0xC3;
            sbuf[1] = (char)total;
            FUN_0053cc30((int)(sbuf + 2), pkt + payStart, payLen);
            int sent = 0, rem = total;
            if (DAT_055ca168 != 0xffffffff) {
                do {
                    int n = send(DAT_055ca168, sbuf + sent, rem - sent, 0);
                    if (n == -1) {
                        int err = WSAGetLastError();
                        if (err == WSAEWOULDBLOCK && (int)(DAT_055cc16c + total) < 0x2001) {
                            memcpy(DAT_055ca16c + DAT_055cc16c, sbuf, total);
                            DAT_055cc16c += total;
                        } else {
                            Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                        }
                        goto send_done;
                    }
                    if (n == 0) break;
                    if (DAT_055ce174) FUN_0043de60();
                    sent += n; rem -= n;
                } while (rem > 0);
            }
        } else {
            // Large packet: [0xC4][len_hi][len_lo][payload...]
            char lbuf[1028];
            int  total = crc + 3;
            lbuf[0] = (char)0xC4;
            lbuf[1] = (char)((total + ((total >> 31) & 0xff)) >> 8);
            lbuf[2] = (char)total;
            FUN_0053cc30((int)(lbuf + 3), pkt + payStart, payLen);
            int sent = 0, rem = total;
            if (DAT_055ca168 != 0xffffffff) {
                do {
                    int n = send(DAT_055ca168, lbuf + sent, rem - sent, 0);
                    if (n == -1) {
                        int err = WSAGetLastError();
                        if (err == WSAEWOULDBLOCK && (int)(DAT_055cc16c + total) < 0x2001) {
                            memcpy(DAT_055ca16c + DAT_055cc16c, lbuf, total);
                            DAT_055cc16c += total;
                        } else {
                            Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                        }
                        goto send_done;
                    }
                    if (n == 0) break;
                    if (DAT_055ce174) FUN_0043de60();
                    sent += n; rem -= n;
                } while (rem > 0);
            }
        }
send_done:
        OpenGL_Release();
    }
    #endif  // re-auth packet block disabled for MuEmu

    // ── KEEPALIVE COUNTER DECREMENT + RE-RANDOMIZE ────────────────────────────
    // Decrements DAT_055ca028 ref-count; when it hits 0 re-randomizes it
    // using a 16-byte XOR key (DAT_00559050). This is anti-tamper obfuscation.
    {
        // Look up the value in the hash table (two lookups: one for zero key, one for value)
        // Simplified: decrement the tracked ref-count
        unsigned idx = HashTable_GetIndex(&DAT_055c9bc8, &DAT_055ca028);
        if (idx != 0xffffffff) {
            int* node = *(int**)(DAT_055c9bcc + idx * 4);
            ((char*)node)[4]--;
            if (((char*)node)[4] == '\0') {
                // Re-randomize DAT_055ca028 with: (+0x47) ^ key[i&0xf] + 0x23 ^ next_byte
                int* tmp = (int*)operator_new(4);
                *tmp = DAT_055ca028;
                for (unsigned i = 0; i < 4; i++) {
                    char* b = (char*)tmp + i;
                    *b = *b + 'G';
                    *b = (char)(*b ^ DAT_00559050[i & 0xf]);
                    *b = *b + '#';
                    if (i < 3)
                        *(BYTE*)b ^= *((BYTE*)tmp + i + 1);
                    ((char*)&DAT_055ca028)[i] = (char)_rand();
                }
                *node = *tmp;
                operator_delete((BYTE*)tmp);
            }
        }
    }

    DAT_055ca038 = 0;
}
