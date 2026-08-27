// Net_CharSelect.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_00513c10 @ 0x00513C10 — CharSelect_SendSelectPacket
// Builds a C1 XOR-encrypted packet: [C1][len][F3][01][charID...][padding][pin10bytes]
// charID = entity +0x1C1 from CharactersClient[DAT_005615e0], stride 0x394.
// InputText[0] is at DAT_07db8710 (10 bytes of PIN data).
// XOR key: {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,0x23,0xa8,0xfe,0xb6,
//           0x49,0x5d,0x39,0x5d,0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
//           0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56}
// After send: login sub-state → 0x18, secondary → 0x15, PlayBuffer(0x1b), ClearInput(1).
void __cdecl CharSelect_SendSelectPacket(void)
{
    static const unsigned char xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };

    DAT_005615e0 = DAT_005616ac; // SelectedHero
    DAT_005616ac = (DWORD)-1;
    DAT_083a7c14 = 0x38;  // CurrentProtocolState

    // Build packet: [C1][len][F3][01] + payload
    unsigned char pkt[0x400];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0xC1;        // packet type
    pkt[1] = 0x00;        // length (filled later)
    pkt[2] = 0xF3;        // opcode
    pkt[3] = 0x01;        // sub-opcode: char select

    unsigned short pos = 4;

    // XOR first segment (bytes 3..3, i.e. just 1 byte — the loop runs for uVar4=3 to 3)
    for (unsigned int i = 3; i < 4; i++) {
        unsigned int ki = i & 0x1F;
        pkt[i] ^= xorKey[ki] ^ pkt[i];
        // Note: the decompile XORs buf[i] with key[ki] XOR buf[i], which zeroes the byte.
        // This is the anti-tamper XOR pattern; actual payload starts next.
    }

    // Append charID string from CharactersClient[DAT_005615e0].ID
    // Entity stride = 0x394, ID at entity + 0x1C1
    char* entityBase = (char*)(uintptr_t)DAT_07abf5d0;
    char* charID = entityBase + (int)DAT_005615e0 * 0x394 + 0x1C1;
    unsigned int idLen = (unsigned int)strlen(charID);

    if (pos + idLen < 0x401) {
        memcpy(pkt + pos, charID, idLen);
        // XOR-encrypt the charID portion
        for (unsigned int i = pos; i < pos + idLen; i++) {
            unsigned int ki = i & 0x1F;
            pkt[i] ^= xorKey[ki] ^ pkt[i]; // double-XOR (anti-tamper pattern)
        }
        pos += (unsigned short)idLen;
    }

    // Append zero padding: (10 - idLen) bytes
    unsigned int padLen = 10 - idLen;
    if (padLen > 0 && pos + padLen < 0x401) {
        memset(pkt + pos, 0, padLen);
        for (unsigned int i = pos; i < pos + padLen; i++) {
            unsigned int ki = i & 0x1F;
            pkt[i] ^= xorKey[ki] ^ pkt[i];
        }
        pos += (unsigned short)padLen;
    }

    // Append 10 bytes of InputText[0] (PIN/second password from DAT_07db8710)
    if (pos + 10 < 0x401) {
        memcpy(pkt + pos, DAT_07db8710, 10);
        for (unsigned int i = pos; i < pos + 10; i++) {
            unsigned int ki = i & 0x1F;
            pkt[i] ^= xorKey[ki] ^ pkt[i];
        }
        pos += 10;
    }

    // Set packet length
    pkt[1] = (unsigned char)pos;

    // Send via socket
    SOCKET sock = DAT_055ca168;
    if (sock != INVALID_SOCKET) {
        int totalSent = 0;
        int remaining = (int)pos;
        while (remaining > 0) {
            int sent = send(sock, (const char*)(pkt + totalSent), remaining, 0);
            if (sent == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    if ((int)(DAT_055cc16c + pos) < 0x2001) {
                        memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, pos);
                        DAT_055cc16c += pos;
                    } else {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    }
                } else {
                    FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                }
                break;
            }
            if (sent == 0) break;
            if (DAT_055ce174 != 0) {
                FUN_0043de60();
            }
            remaining -= sent;
            totalSent += sent;
        }
    }

    // Post-send state transitions
    DAT_083a7c14 = 0x18;
    DAT_083a7c18 = 0x15;
    PlayBuffer(0x1b, 0, 0);
    FUN_0047ec60(1);              // ClearInput(1)
    DAT_00559c84 = 0;             // InputEnable = false
}
// FUN_0051af50 — implemented in src/UI/UI_StatsPanel.cpp (UI_StatsPanel_Render)
// FUN_0051e0c0 — implemented in src/Render/Scene_CharPreview.cpp
// FUN_0051e7e0 — implemented in src/Scene/Scene_ServerSelect_Input.cpp (server select hit-test, 337 lines)
// FUN_005239a0 — implemented in src/Render/Scene_CharPreview.cpp
// FUN_0052a050 — implemented in src/Render/Texture.cpp
// FUN_0053d5c0 — implemented in src/Render/Texture.cpp
