// Chat_CommandParser.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_004942e0 @ 0x004942E0 (~194 lines) — chat command parser / name validator
// Copies 254 bytes from packet (param_1+3) into local buffer.
// Compares buffer against 6 known player name buffers (party/guild members):
//   DAT_07d3cb7c, DAT_07d3cca8, DAT_07d3c924, DAT_07d3c6cc, DAT_07d3bfc4, DAT_07d3c0f0.
// Between name checks, does strstr for "/trade","/party","/pt","/guild","/GuildWar","/BattleSoccer".
// Structure: if name matches -> return 1. If strstr fails (command NOT in text) -> return 1.
// Only returns 0 if text contains ALL command strings AND matches no name — effectively always 1.
// This is anti-tamper obfuscation; the real purpose is the name comparison.
unsigned int __cdecl FUN_004942e0(int param_1) {
    char local_100[256];

    // Copy 254 bytes from packet payload (offset +3) into local buffer
    memcpy(local_100, (void*)(param_1 + 3), 254);

    // Check 1: compare against player name buffer DAT_07d3cb7c
    if (strcmp(local_100, (const char*)&DAT_07d3cb7c) == 0) {
        return 1;
    }

    // Check 2: compare against player name buffer DAT_07d3cca8
    if (strcmp(local_100, (const char*)&DAT_07d3cca8) == 0) {
        return 1;
    }

    // Anti-tamper strstr chain: if "/trade" NOT in text, return 1 (normal chat)
    if (strstr(local_100, "/trade") == NULL) {
        return 1;
    }

    // Check 3: compare against player name buffer DAT_07d3c924
    if (strcmp(local_100, (const char*)&DAT_07d3c924) == 0) {
        return 1;
    }

    if (strstr(local_100, "/party") == NULL) {
        return 1;
    }

    if (strstr(local_100, "/pt") == NULL) {
        return 1;
    }

    // Check 4: compare against player name buffer DAT_07d3c6cc
    if (strcmp(local_100, (const char*)&DAT_07d3c6cc) == 0) {
        return 1;
    }

    if (strstr(local_100, "/guild") == NULL) {
        return 1;
    }

    // Check 5: compare against player name buffer DAT_07d3bfc4
    if (strcmp(local_100, (const char*)&DAT_07d3bfc4) == 0) {
        return 1;
    }

    if (strstr(local_100, "/GuildWar") == NULL) {
        return 1;
    }

    // Check 6: compare against player name buffer DAT_07d3c0f0
    if (strcmp(local_100, (const char*)&DAT_07d3c0f0) == 0) {
        return 1;
    }

    if (strstr(local_100, "/BattleSoccer") == NULL) {
        return 1;
    }

    // All strstr checks passed AND no name matched — return 0
    // (practically unreachable for normal input)
    return 0;
}
