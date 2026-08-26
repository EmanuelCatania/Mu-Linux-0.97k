// Chat_Send.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

// SendChat @ 0x004C1B90 (~454 lines) — SUMMARY STUB
// Sends a chat message packet to server.
// Flow:
//   1. Rate-limit: if ChatTime > 0x32, return
//   2. Duplicate check: compare with last sent text (DAT_05826adc)
//   3. Set ChatTime = 0x46, copy text to last-sent buffer
//   4. Check if Hero->ID contains "webzen" substring (GM detection)
//   5. Build C1 packet with XOR encryption using 32-byte key
//   6. Net_Send the encrypted chat packet
void __cdecl SendChat_stub(char* Text) {
    // 0x004C1B90 — Sends chat message packet to server (454 lines decompiled).
    // ~60% of original is anti-tamper XOR key init + dead forward/reverse loops.
    // Real logic:
    //   1. Rate-limit: if ChatTime > 0x32, return
    //   2. Duplicate check vs last-sent buffer (DAT_05826adc)
    //   3. Set ChatTime=0x46, copy text to last-sent buffer
    //   4. GM name check: if Hero->ID contains "webzen", skip chat (anti-impersonation)
    //   5. Dead check: if Hero->Dead, check if Text starts with GM command (DAT_07d3cdd4)
    //   6. Whisper block commands: "/whisper" → toggle m_bBlockWhisper_1
    //   7. Build C1 packet: [C1][len][01][heroID 10 bytes][text up to 60 bytes]
    //   8. XOR-encrypt payload with 32-byte key, fix header, send via socket

    // Rate limit — `ChatTime` es el GLOBAL 0x05826D08 (= DAT_05826d08), que
    // `Game_MainLoop` decrementa un tick por frame (IDA 0x5262D9-0x5262EA):
    //     if ( ChatTime > 0 ) --ChatTime;
    //
    // 2026-08-12 BUG-FIX: acá era un `static int s_ChatTime` local, y NADIE lo
    // decrementaba. Después del primer mensaje quedaba clavado en 70, así que
    // `if (> 50) return` bloqueaba **todo** el chat posterior de la sesión.
    // Síntoma reportado: el primer `/move <mapa>` funcionaba y los siguientes
    // no hacían nada — el jugador se quedaba en el destino del primero y
    // parecía que el comando "recordaba" el mapa anterior.
    // Lo usan además WndProc y Chat_InputTick, que ya leían el global real.
    if ((int)DAT_05826d08 > 0x32) return;

    // Duplicate check: compare with last-sent text
    if ((int)DAT_05826d08 > 0 && strcmp(DAT_05826adc, Text) == 0) return;

    // Set cooldown and copy to last-sent buffer
    DAT_05826d08 = 0x46;
    strncpy(DAT_05826adc, Text, sizeof(DAT_05826adc) - 1);
    DAT_05826adc[sizeof(DAT_05826adc) - 1] = '\0';

    // GM name check: if Hero->ID contains "webzen" substring, block
    // Hero is char* (DAT_07abf5d8), ID at offset 0x1C1
    {
        const char* heroId = Hero + 0x1C1;
        const char* webzenStr = &DAT_005592dc;  // "webzen" string in binary
        int webzenLen = (int)strlen(webzenStr);
        if (webzenLen > 0) {
            int heroIdLen = (int)strlen(heroId);
            int diff = heroIdLen - webzenLen;
            if (diff >= 0) {
                for (int i = 0; i <= diff; i++) {
                    if (strncmp(heroId + i, webzenStr, webzenLen) == 0) {
                        // Found "webzen" in hero name — block chat (anti-impersonation)
                        return;
                    }
                }
            }
        }
    }

    // If hero is dead, check for GM command prefix
    // Dead at offset 0x2FD in CHARACTER struct
    if (*(BYTE*)(Hero + 0x2FD) != '\0') {
        // Only allow GM command (DAT_07d3cdd4) when dead
        int cmdLen = (int)strlen(&DAT_07d3cdd4);
        if (cmdLen > 0 && strncmp(Text, &DAT_07d3cdd4, cmdLen) == 0) {
            return;  // block GM commands from dead players
        }
    }
    else {
        // Alive: check whisper block toggle commands
        if (Text[0] == '/') {
            // Check "/whisperblock on" command (DAT_07d3d284)
            int len1 = (int)strlen(&DAT_07d3d284);
            if (len1 > 0 && strncmp(Text, &DAT_07d3d284, len1) == 0) {
                // `m_bBlockWhisper_1` es el GLOBAL 0x07E11DAC (= DAT_07e11dac),
                // que ya leen Net_Process (F3 toggle) y Chat_InputTick.
                // 2026-08-12 BUG-FIX: acá había DOS `static bool` distintos —
                // uno para "on" y otro para "off" — así que el toggle no
                // cambiaba nada y nadie podía leer el estado.
                DAT_07e11dac = 1;
                UIChatLogWindow_AddText((const char*)&lpDefault_00583d88, &DAT_07d3d608, 1);
                return;
            }
            // Check "/whisperblock off" command (DAT_07d3d3b0)
            int len2 = (int)strlen(&DAT_07d3d3b0);
            if (len2 > 0 && strncmp(Text, &DAT_07d3d3b0, len2) == 0) {
                DAT_07e11dac = 0;   // mismo global que el "on" (IDA L129/L135)
                UIChatLogWindow_AddText((const char*)&lpDefault_00583d88, &DAT_07d3d734, 1);
                return;
            }
        }
    }

    // Build chat packet
    BYTE pkt[0x400];
    memset(pkt, 0, sizeof(pkt));
    int pos = 0;

    pkt[pos++] = 0xC1;  // packet type
    pkt[pos++] = 0;     // length placeholder
    // 004C1B90 inicializa el byte de cabecera del chat con 0 (buf[4]); el
    // GameServer despacha PMSG_CHAT_RECV por head 0x00.  0x01 pertenece al
    // byte auxiliar de la pila de IDA, no al paquete transmitido.
    pkt[pos++] = 0x00;  // opcode: chat (IDA: C1 <len> 00)

    // Copy Hero->ID (10 bytes) — ID at offset 0x1C1 in CHARACTER struct
    memcpy(pkt + pos, Hero + 0x1C1, 10);
    pos += 10;  // pos = 13

    // Copy chat text (max 60 bytes including null)
    int textLen = (int)strlen(Text) + 1;
    if (textLen > 0x3c) textLen = 0x3c;
    memcpy(pkt + pos, Text, textLen);
    pos += textLen;

    short totalLen = (short)pos;

    // XOR-encrypt payload bytes 3..pos with 32-byte key (chained XOR)
    static const BYTE xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };
    // IDA 004C1B90 encrypts each payload byte against the PREVIOUS
    // transmitted byte (the opcode for the first character), in ascending
    // order.  Using pkt[i+1] here changes the command text before MuEmu sees
    // it, which is especially visible with /move destinations.
    // Encrypt hero ID region (bytes 3..12).
    for (int i = 3; i < 13 && i < pos; i++) {
        pkt[i] ^= xorKey[i & 0x1f] ^ pkt[i - 1];
    }
    // Encrypt text region (bytes 13..pos)
    for (int i = 13; i < pos; i++) {
        pkt[i] ^= xorKey[i & 0x1f] ^ pkt[i - 1];
    }

    // Fix header length
    pkt[1] = (BYTE)totalLen;

    // Send via socket — DAT_055ca168 is the socket handle
    SOCKET sock = (SOCKET)DAT_055ca168;
    if (sock != INVALID_SOCKET) {
        int sent = 0;
        int toSend = (int)(totalLen & 0xffff);
        while (sent < toSend) {
            int ret = send(sock, (char*)(pkt + sent), toSend - sent, 0);
            if (ret == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK) {
                    FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    return;
                }
                if ((int)(DAT_055cc16c + (DWORD)toSend) > 0x2000) {
                    FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    return;
                }
                memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, toSend);
                DAT_055cc16c += (DWORD)toSend;
                return;
            }
            if (ret == 0) return;
            if (DAT_055ce174 != 0) FUN_0043de60();
            sent += ret;
        }
    }
}
