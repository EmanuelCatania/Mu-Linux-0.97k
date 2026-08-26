#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_0047a010 @ 0x0047A010 — Gate_LoadData(path) ─────────────────────────
// Reads text-format gate/warp data file (Data2/Gate.txt Korean locale).
// Parser uses TextParser_GetToken (type 0=comment/END, 1=record, 2=EOF).
// Gate data structure: DAT_07cf5600 + gate_id * 9 bytes (stride 9).
//   Each 9-byte record stores: [0]=flag, [1]=mapid, [2]=x, [3]=y,
//     [4]=dst_mapid, [5]=dst_x, [6]=dst_y, [7]=size, [8]=dir.
// Total 100 entries (900 bytes, loop runs while off < 900).
void __cdecl FUN_0047a010(const char *path)
{
    DAT_07d7806c = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

    do {
        while (true) {
            int recType = TextParser_GetToken();
            if (recType == 2) goto EOF_done;
            if (recType == 0) break;   // compare with "END"; break if matched
            if (recType == 1) {
                long long lv = 0; (void)lv;
                // gate_id = (int)__ftol() from TextParser_GetToken
                // Then read 9 fields sequentially; each TextParser_GetToken call
                // returns the next token as float; cast to byte.
                // ptr = &DAT_07cf5600 + gate_id * 9
                // ptr[0..8] = 9 successive field reads
            }
        }
        // Break out if current token string == "END"
    } while (true);

EOF_done:
    FUN_0054150f(DAT_07d7806c);
}

// ── FUN_0047a170 @ 0x0047A170 — Gate_SaveBMD(path) ──────────────────────────
// Writes gate data table to a binary .bmd file (no checksum).
// Allocates 9-byte scratch buffer, iterates 100 entries (stride 9),
// XOR-encrypts each 9-byte record via FUN_00479910, writes via FUN_005430f0.
void __cdecl FUN_0047a170(const char *path)
{
    FILE *fp  = (FILE *)FUN_0054173f(path, DAT_005597d4);  // "wb"
    char *buf = (char *)operator_new(9);
    int off   = 0;
    do {
        // copy 9 bytes from gate table
        *(DWORD *)buf          = *(DWORD *)((char *)DAT_07cf5600 + off);
        *(DWORD *)(buf + 4)    = *(DWORD *)((char *)DAT_07cf5600 + off + 4);
        buf[8]                 = *((char *)DAT_07cf5600 + off + 8);
        FUN_00479910((int)buf, 9);
        FUN_005430f0(buf, 9, 1, (int *)fp);
        off += 9;
    } while (off < 900);
    operator_delete(buf);
    FUN_0054150f(fp);
}

// ── FUN_0047a4d0 @ 0x0047A4D0 — Gate_LoadBMD(path) ──────────────────────────
// Reads binary .bmd gate data (counterpart to FUN_0047a170).
// Allocates 9-byte scratch buffer, reads 100 records sequentially,
// XOR-decrypts each via FUN_00479910, copies into DAT_07cf5600 (stride 9).
void __cdecl FUN_0047a4d0(const char *path)
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
    char *buf = (char *)operator_new(9);
    int off   = 0;
    do {
        FUN_00541597(buf, 9, 1, (int *)fp);
        FUN_00479910((int)buf, 9);
        DWORD *dst = (DWORD *)((char *)DAT_07cf5600 + off);
        off += 9;
        dst[0] = *(DWORD *)buf;
        dst[1] = *(DWORD *)(buf + 4);
        *(char *)(dst + 2) = buf[8];
    } while (off < 900);
    operator_delete(buf);
    FUN_0054150f(fp);
}
