#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_0047a5b0 @ 0x0047A5B0 — Skill_LoadData(path) ────────────────────────
// Reads a text-format skill data file.
// Parser uses FUN_0047a1f0 (record type 0/1/2):
//   0 = section separator — strcmp with "END"; break inner loop
//   1 = record: lVar21 = skill_id (×0x28); reads name string into
//       DAT_07d29d20 + skill_id*0x28, then fields:
//         [0x20] = type byte
//         [0x21] = target byte
//         [0x22..0x23] = short mana
//         [0x24..0x25] = short dmg
//         [0x26] = cooldown
//         [0x27] = range
//       Inserts/updates entry in hash table at DAT_055c9bc8.
//       Hash table ops use FUN_00403f80 (insert), HashTable_GetIndex,
//       FUN_0047ea70 (serialize+insert), FUN_0047eaf0 (free+remove).
//   2 = EOF — close file, return
//
// Hash table context: DAT_055c9bc8 (vtable), DAT_055c9bd0 (bucket array),
//   DAT_055c9bcc (value array), DAT_055c9bd4 (bucket count).
// Each skill entry is 0x29 bytes; [0x28] = 1 (alive flag).
void __cdecl FUN_0047a5b0(const char *path)
{
    DAT_07d7806c = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

    // Streaming text parser — logic mirrors Ghidra decompile of FUN_0047a5b0.
    // The real loop calls FUN_0047a1f0 to tokenize floats/strings from the
    // open file handle DAT_07d7806c, using DAT_07cf1ef0 as a temp string buf.
    // Skill records are stored at DAT_07d29d20 + index*0x28 (stride 0x28).
    // Each record written is also registered in the hash table (DAT_055c9bc8).
    // Stub: call parser until EOF.
    while (FUN_0047a1f0() != 2) { /* parse handled by FUN_0047a1f0 side-effects */ }
    FUN_0054150f(DAT_07d7806c);
}

// ── FUN_0047a970 @ 0x0047A970 — Skill_SaveBMD(path) ─────────────────────────
// Writes skill data table to a binary .bmd file.
// Allocates 0xa00-byte buffer (40 skill entries × 0x28 bytes each).
// For each entry: copies from DAT_07d29d20, XOR-encrypts via FUN_00479910,
// writes to file. After all entries, computes a rolling checksum
// (seed: DAT_00b43000, constant 0x5a18) and appends 4 bytes.
// Hash table operations (FUN_0047ea70 / FUN_0047eaf0) manage in-memory index.
void __cdecl FUN_0047a970(const char *path)
{
    FILE *fp   = (FILE *)FUN_0054173f(path, DAT_005597d4);  // "wb"
    char *buf  = (char *)operator_new(0xa00);
    const char *src = (const char *)DAT_07d29d20;
    char *p = buf;
    for (const char *end = src + 0xa00; src < end; src += 0x28, p += 0x28) {
        memcpy(p, src, 0x28);
        FUN_00479910((int)p, 0x28);
    }
    FUN_005430f0(buf, 0xa00, 1, (int *)fp);
    DWORD cs = DAT_00b43000;
    for (UINT i = 0; i < 0x9fd; i += 4) {
        DWORD v  = *(DWORD *)(buf + i);
        DWORD q  = (i >> 2) & 1;
        if      (q == 0) cs ^= v;
        else if (q == 1) cs += v;
        if ((i & 0xf) == 0)
            cs ^= (DWORD)(cs + 0x5a18) >> (((BYTE)(i >> 2) & 7) + 1);
    }
    FUN_005430f0((char *)&cs, 4, 1, (int *)fp);
    operator_delete(buf);
    FUN_0054150f(fp);
}

// ── FUN_0047ac50 @ 0x0047AC50 — Skill_LoadBMD(path) ─────────────────────────
// Reads binary .bmd skill data file (counterpart to FUN_0047a970).
// Reads 0xa00 bytes + 4-byte checksum, validates, XOR-decrypts each 0x28-byte
// record, and writes into DAT_07cf1ff8 (skill shadow table, stride 0x28).
// Hash table entries are rebuilt via FUN_0047ea70 / FUN_0047eaf0.
// [+0x26] field in shadow table is left-shifted by 1 after copy.
void __cdecl FUN_0047ac50(const char *path)
{
    CHAR msg[256];
    FILE *fp = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!fp) {
        crt_sprintf(msg, (const char *)s__s___File_not_exist__00558094);
        FUN_00405540(&DAT_055c9bf0, msg);
        MessageBoxA(DAT_055c9ffc, msg, nullptr, 0);
        SendMessageA(DAT_055c9ffc, 2, 0, 0);
        return;
    }
    char *buf0 = (char *)operator_new(0xa00);
    char *buf  = buf0;
    FUN_00541597(buf, 0xa00, 1, (int *)fp);
    DWORD stored_cs;
    FUN_00541597((char *)&stored_cs, 4, 1, (int *)fp);
    FUN_0054150f(fp);

    DWORD cs = DAT_00b43000;
    for (UINT i = 0; i < 0x9fd; i += 4) {
        DWORD v  = *(DWORD *)(buf + i);
        DWORD q  = (i >> 2) & 1;
        if      (q == 0) cs ^= v;
        else if (q == 1) cs += v;
        if ((i & 0xf) == 0)
            cs ^= (DWORD)(cs + 0x5a18) >> (((BYTE)(i >> 2) & 7) + 1);
    }
    if (stored_cs != cs) {
        crt_sprintf(msg, (const char *)s__s___File_corrupted__00559bd4);
        FUN_00405540(&DAT_055c9bf0, msg);
        MessageBoxA(DAT_055c9ffc, msg, nullptr, 0);
        SendMessageA(DAT_055c9ffc, 2, 0, 0);
        operator_delete(buf0);
        return;
    }

    int off = 0;
    do {
        FUN_00479910((int)buf, 0x28);
        // copy decrypted record into skill shadow table
        memcpy((char *)DAT_07cf1ff8 + off, buf, 0x28);
        // [+0x26] field shift
        *((BYTE *)DAT_07cf1ff8 + off + 0x26) <<= 1;
        off += 0x28;
        buf += 0x28;
    } while (off <= 0x9ff);
    operator_delete(buf0);
}
